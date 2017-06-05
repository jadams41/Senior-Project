#include "memoryManager.h"
#include <stdint-gcc.h>
#include "printk.h"
#include "utils.h"

extern void store_control_registers();
extern uint64_t saved_cr2;
extern uint64_t saved_cr3;

memory_info memInfo;

void init_usable_segment_struct(){
    int i;
    for(i = 0; i < 20; i++){
        usable_memory_segment *seg = &(memInfo.segments[i]);
        seg->beg_addr = 0;
        seg->end_addr = 0;
        seg->length = 0;
        seg->position_inside_segment = 0;

        blocked_memory_segment *bseg = &(memInfo.blocked_segments[i]);
        bseg->beg_addr = -1;
        bseg->end_addr = -1;
    }
    memInfo.seg_length = 0;
    memInfo.bseg_length = 0;
    memInfo.curSegment = -1;

    memInfo.node_frame.beg_addr = 0;
    memInfo.node_frame.end_addr = 0;
    memInfo.node_frame.current_position_in_frame = 0;

    memInfo.free_frames_list = 0;
    memInfo.used_frames_list = 0;

    memInfo.end_of_memory = 0;
}

void update_end_of_memory(uint64_t new_end){
    if(new_end > memInfo.end_of_memory){
        memInfo.end_of_memory = new_end;
    }
}

//todo, check if segment is smaller than 4K and if so don't add
void add_segment(uint64_t beg, uint64_t end, uint64_t len){
    memory_info *info = &memInfo;
    if(info->seg_length == 19){
        printk_err("segment array out of space\n");
        return;
    }

    if(beg == 0){
        printk_err("received a segment beginning at 0, probably invalid\n");
        return;
    }

    uint32_t length = info->seg_length;
    info->segments[length].beg_addr = beg;
    info->segments[length].end_addr = end;
    info->segments[length].length = len;
    info->segments[length].position_inside_segment = beg;

    if(info->curSegment == -1) info->curSegment = length;
    info->seg_length += 1;

}

void add_blocked_segment(uint64_t beg, uint64_t end){
    memory_info *info = &memInfo;
    if(info->bseg_length == 19){
        printk_err("blocked segment array out of space\n");
        return;
    }

    uint32_t length = info->bseg_length;
    info->blocked_segments[length].beg_addr = beg;
    info->blocked_segments[length].end_addr = end;

    info->bseg_length += 1;
}

//return end address of blocked_segment if the passed in address is in any of the blocked regions, otherwise 0
static uint64_t address_in_blocked_memory(uint64_t addr){
    memory_info *info = &memInfo;

    int i;
    for(i = 0; i < info->bseg_length; i++){
        blocked_memory_segment *bseg = info->blocked_segments + i;
        if(addr >= bseg->beg_addr && addr <= bseg->end_addr){
            return bseg->end_addr;
        }
    }
    return 0;
}

//returns 0 (NULL) if out of memory, otherwise adress of next available frame
static int grab_next_frame(){
    memory_info *info = &memInfo;
    int end_of_blocked_memory;
    if(info->curSegment == -1 || info->curSegment == (int)info->seg_length){
        //out of memory
        return 0;
    }
    usable_memory_segment *curSeg = info->segments + info->curSegment;
    while(1){
        int possible_next_frame_addr = curSeg->position_inside_segment;
        while(possible_next_frame_addr % 4096) possible_next_frame_addr += 1;
        //check if there is a 4k frame inside of the currentSegment
        if((possible_next_frame_addr + 4096) > curSeg->end_addr){
            //go to the next memory segments
            if(info->curSegment == -1 || info->curSegment == (int)info->seg_length){
                //out of memory
                return 0;
            }
            curSeg = info->segments + (++(info->curSegment));
        }
        else if((end_of_blocked_memory = address_in_blocked_memory(possible_next_frame_addr)) || address_in_blocked_memory(possible_next_frame_addr + 4096)){
            if(address_in_blocked_memory(possible_next_frame_addr + 4096)){
                end_of_blocked_memory = address_in_blocked_memory(possible_next_frame_addr + 4096);
            }
            if(end_of_blocked_memory < (curSeg->end_addr + 1)){
                curSeg->position_inside_segment = end_of_blocked_memory + 1;
            }
            else {
                curSeg = info->segments + (++(info->curSegment));
            }
        }
        else {
            curSeg->position_inside_segment = possible_next_frame_addr + 4096;
            return possible_next_frame_addr;
        }
    }
    return 0;
}

//this method should be called when the pf linked list is empty
//NOTE: this will happen when the first frame is requested
//NOTE: this adds 20 page frames to the linked list at a time
//returns 1 if out of memory (or no segments have been initialized yet)
static int populate_pf_list(){
    memory_info *info = &memInfo;
    int numSegmentsAdded = 0;
    uint64_t next_frame_addr;
    frame_list_node *new_node, *lastNode = 0;

    while(numSegmentsAdded < 20){
        //check if we need another frame for the list nodes
        if(info->node_frame.end_addr - info->node_frame.current_position_in_frame < sizeof(frame_list_node)){
            //grab another frame for the nodes list
            next_frame_addr = grab_next_frame(info);
            if(next_frame_addr == 0){
                return 1;
            }
            info->node_frame.beg_addr = info->node_frame.current_position_in_frame = next_frame_addr;
            info->node_frame.end_addr = info->node_frame.beg_addr + 4096;
        }
        next_frame_addr = grab_next_frame(info);
        if(!next_frame_addr) return 1;

        new_node = (frame_list_node*)info->node_frame.current_position_in_frame;
        new_node->used = 0;
        new_node->beg_addr = next_frame_addr;
        new_node->end_addr = new_node->beg_addr + 4096;
        new_node->prev_frame = lastNode;
        new_node->next_frame = 0;

        lastNode->next_frame = new_node;
        lastNode = new_node;

        if(info->free_frames_list == 0){
            info->free_frames_list = new_node;
        }

        info->node_frame.current_position_in_frame += sizeof(*new_node);
        numSegmentsAdded++;
    }

    return 0;
}

int count = 0;
//will attempt to grab a page frame off of the page frame list
//then will add a
void *MMU_pf_alloc(){
    memory_info *info = &memInfo;
    int err;
    if(info->free_frames_list == 0){
        err = populate_pf_list(info);
        if(err){
            printk("There was an error populating the page frame list\n");
            return 0;
        }
    }

    frame_list_node *toAlloc = info->free_frames_list;
    if(toAlloc == 0){
        printk_err("there was an error allocating a frame\n");
        return 0;
    }
    toAlloc->used = 1;

    info->free_frames_list = toAlloc->next_frame;

    if(info->free_frames_list){
        info->free_frames_list->prev_frame = 0;
    }

    toAlloc->next_frame = info->used_frames_list;
    toAlloc->prev_frame = 0;

    if(info->used_frames_list != 0)
        info->used_frames_list->prev_frame = toAlloc;
    info->used_frames_list = toAlloc;

    count++;
    if(count % 1600 == 0)
        printk_warn("apx alloc'd %d%% of phys pages\n", (count * 100/32000));

    return (void*)toAlloc->beg_addr;
}

void MMU_pf_free(void *pf){
    memory_info *info = &memInfo;
    frame_list_node *curUsedNode = info->used_frames_list;
    while(1){
        if(curUsedNode == 0){
            printk_err("tried to free a node that wasn't allocated\n");
            return;
        }
        if(curUsedNode->beg_addr == (uint64_t)pf){
            //mark this node as unused
            curUsedNode->used = 0;

            //take this node off of the free node list
            if(curUsedNode->prev_frame != 0){
                curUsedNode->prev_frame->next_frame = curUsedNode->next_frame;
            }
            else {
                info->used_frames_list = curUsedNode->next_frame;
            }
            if(curUsedNode->next_frame != 0){
                curUsedNode->next_frame->prev_frame = curUsedNode->prev_frame;
            }

            //move this node to the beginning of the free list
            curUsedNode->prev_frame = 0;
            curUsedNode->next_frame = info->free_frames_list;
            if(info->free_frames_list != 0){
                info->free_frames_list->prev_frame = curUsedNode;
            }
            info->free_frames_list = curUsedNode;
            return;
        }
        curUsedNode = curUsedNode->next_frame;
    }
}

void zero_out_page(void *pf){
    uint64_t *position_in_page = (uint64_t*)pf;
    int i;
    for(i = 0; i < 512; i++){
        position_in_page[i] = 0;
    }
}

/***** BEGIN VIRTUAL MEMORY FUNCTIONS *****/
static void initialize_kernel_heap(void *pageTable){
    uint64_t *tableAccess = (uint64_t*)pageTable, *new_page;

    //check that the kernel heap PML4E hasn't already been set
    if(tableAccess[15] != 0){
        printk_err("kernel heap is already initialized\n");
        return;
    }

    //create the page to hold the kernel heap PDPT
    new_page = (uint64_t*) MMU_pf_alloc();
    zero_out_page(new_page);

    tableAccess[15] = ((uint64_t)new_page) | 0b11;
}

static void initialize_user_space(void *pageTable){
    uint64_t *tableAccess = (uint64_t*)pageTable, *new_page;

    //check that the kernel heap PML4E hasn't already been set
    if(tableAccess[16] != 0){
        printk_err("kernel heap is already initialized\n");
        return;
    }

    //create the page to hold the kernel heap PDPT
    new_page = (uint64_t*) MMU_pf_alloc();
    zero_out_page(new_page);

    tableAccess[16] = ((uint64_t)new_page) | 0b11;

}

/**
  * allocates physical pages to hold the identity map portion of the page table
  * returns the pointer to the bottom of the p4 table
  */
void *init_page_table(){
    void *p4_table, *p3_table, *p2_table, *p1_table;
    //grab a frame for the highest level of the page frame table
    //this will also be the address returned
    p4_table = MMU_pf_alloc();
    zero_out_page(p4_table);

    uint64_t *p4_access = (uint64_t*)p4_table, *p3_access, *p2_access, *p1_access;
    printk("new page table will be at %lx\n", p4_access);

    //create the identity mapped region
    p3_table = MMU_pf_alloc();
    zero_out_page(p3_table);

    p4_access[0] = (uint64_t)p3_table | 0b11;
    printk("first entry of p4 table points to %lx\n", *p4_access);

    uint64_t current_real_address = 0;
    int p3_idx = 0, p2_idx = 0, p1_idx = 0;
    int done = 0;

    p3_access = (uint64_t*)p3_table;
    while(!done){
        //set the p3 entry to a new p2 table
        p2_table = MMU_pf_alloc();
        zero_out_page(p2_table);
        p3_access[p3_idx] = (uint64_t)p2_table | 0b11;
        p2_access = (uint64_t*)p2_table;

        for(p2_idx = 0; p2_idx < 512 && !done; p2_idx++){
            //create a new p1 table
            p1_table = MMU_pf_alloc();
            zero_out_page(p1_table);
            p2_access[p2_idx] = (uint64_t)p1_table | 0b11;
            p1_access = (uint64_t*)p1_table;

            for(p1_idx = 0; p1_idx < 512 && !done; p1_idx++){
                p1_access[p1_idx] = (current_real_address++ * 4096) | 0b11;
                if(current_real_address * 4096 >= memInfo.end_of_memory){
                    done = 1;
                }
            }
        }
        if(++p3_idx >= 512){
            printk_err("somehow ran out of virtual addresses in first p4 entry when trying to make the identity map\n");
            break;
        }
    }
    printk("mapped up to 0x%lx\n", current_real_address * 4096);

    initialize_kernel_heap(p4_table);
    initialize_user_space(p4_table);

    return p4_table;
}

//check the available bits in the entry
//if any of them are set assume that the memory is set,
//else assume that it is free
// static int PTE_free(uint64_t pte){
//     return (pte & 0b111000000011) == 0;
// }

uint64_t user_p4_idx = 16, user_p3_idx = 0, user_p2_idx = 0, user_p1_idx = 0;
uint64_t p4_idx = 15, p3_idx = 0, p2_idx = 0, p1_idx = 0;
void *MMU_alloc_page(){
    store_control_registers();
    uint64_t cr3 = saved_cr3;
    uint64_t *table_accessor = (uint64_t*)cr3, *p3_access, *p2_access, *p1_access;
    void *cur_entry;
    if(!entry_present(table_accessor[p4_idx])){
        printk_err("kernel heap has not been ininitialized\n");
        return 0;
    }

    p3_access = (uint64_t*)table_accessor[p4_idx];
    p3_access = strip_present_bits(p3_access); //remove the 0b11 from the p4 entry to create a valid address
    //iterate over the p3 table, allocating pages for the p2 entries if needed
    p2_access = (uint64_t*)p3_access[p3_idx];
    if(!entry_present((uint64_t)p2_access)){
        cur_entry = MMU_pf_alloc();
        if(!cur_entry) goto VIRTUAL_ALLOC_ERR;

        zero_out_page(cur_entry);
        p2_access = (uint64_t*) cur_entry;
        p3_access[p3_idx] = (uint64_t) p2_access | 0b11;
    }
    else {
        p2_access = strip_present_bits(p2_access); //remove the 0b11 from the entry so that it is a valid address
    }
    p1_access = (uint64_t*)p2_access[p2_idx];
    if(!entry_present((uint64_t)p1_access)){
        cur_entry = MMU_pf_alloc();
        if(!cur_entry) goto VIRTUAL_ALLOC_ERR;

        zero_out_page(cur_entry);
        p1_access = (uint64_t*) cur_entry;
        p2_access[p2_idx] = (uint64_t) p1_access | 0b11;
    }
    else {
        p1_access = strip_present_bits(p1_access); //remove the 0b11 from the entry so that it is a valid address
    }
    p1_access[p1_idx] |= 0b1000000000;
    uint64_t retAddr = p4_idx;
    retAddr <<= 39; //index of the kernel heap in p4
    retAddr |= p3_idx << 30; //index of this page in the p3 table
    retAddr |= p2_idx << 21; //index of this page in the p2 table
    retAddr |= p1_idx << 12; //index of this page in the p1 table
    if(++p1_idx == 512){
        p1_idx = 0;
        if(++p2_idx == 512){
            p2_idx = 0;
            if(++p3_idx == 512) goto VIRTUAL_ALLOC_ERR;
            // print("p3 incremented to %d\n", p3_idx);
        }
    }

    return (void*)retAddr;

    VIRTUAL_ALLOC_ERR:
    printk_err("apparently out of virtual memory in the kernel heap, likely impossible\n");
    return 0;
}

void *MMU_alloc_pages(int num){
    if(num <= 0){
        printk_err("invalid number of pages to allocate, nothing will happen\n");
        return 0;
    }
    int i;
    void *toRet;
    void *pg = toRet = MMU_alloc_page();

    for(i = 1; i < num; i++){
        if(pg == 0){
            printk_warn("ran out of memory while trying to allocate multiple pages. Only allocated %d pages\n", i - 1);
            break;
        }
        pg = MMU_alloc_page();
    }
    return toRet;
}

void *MMU_alloc_user_page(){
    uint64_t saved_p4 = p4_idx;
    uint64_t saved_p3 = p3_idx;
    uint64_t saved_p2 = p2_idx;
    uint64_t saved_p1 = p1_idx;

    p4_idx = user_p4_idx;
    p3_idx = user_p3_idx;
    p2_idx = user_p2_idx;
    p1_idx = user_p1_idx;

    void *toRet = MMU_alloc_page();

    user_p4_idx = p4_idx;
    user_p3_idx = p3_idx;
    user_p2_idx = p2_idx;
    user_p1_idx = p1_idx;

    p4_idx = saved_p4;
    p3_idx = saved_p3;
    p2_idx = saved_p2;
    p1_idx = saved_p1;

    return toRet;
}

void *MMU_alloc_user_pages(int num){
    uint64_t saved_p4 = p4_idx;
    uint64_t saved_p3 = p3_idx;
    uint64_t saved_p2 = p2_idx;
    uint64_t saved_p1 = p1_idx;

    p4_idx = user_p4_idx;
    p3_idx = user_p3_idx;
    p2_idx = user_p2_idx;
    p1_idx = user_p1_idx;

    void *toRet = MMU_alloc_pages(num);

    user_p4_idx = p4_idx;
    user_p3_idx = p3_idx;
    user_p2_idx = p2_idx;
    user_p1_idx = p1_idx;

    p4_idx = saved_p4;
    p3_idx = saved_p3;
    p2_idx = saved_p2;
    p1_idx = saved_p1;

    return toRet;
}

void MMU_free_page(void *vpage){
    uint64_t p4_index, p3_index, p2_index, p1_index, grabber = 0b111111111; //grabber grabs the 9 bit index into the given table
    uint64_t vpage_addr = (uint64_t)vpage;

    grabber <<= 12;
    p1_index = (vpage_addr & grabber) >> 12;
    grabber <<= 9;
    p2_index = (vpage_addr & grabber) >> 21;
    grabber <<= 9;
    p3_index = (vpage_addr & grabber) >> 30;
    grabber <<= 9;
    p4_index = (vpage_addr & grabber) >> 39;

    uint64_t *table_ptr = (uint64_t*)((uint64_t*)saved_cr3)[p4_index];
    if(!entry_present((uint64_t)table_ptr)) goto page_table_error;
    table_ptr = strip_present_bits(table_ptr);

    table_ptr = (uint64_t*)table_ptr[p3_index];
    if(!entry_present((uint64_t)table_ptr)) goto page_table_error;
    table_ptr = strip_present_bits(table_ptr);

    table_ptr = (uint64_t*)table_ptr[p2_index];
    if(!entry_present((uint64_t)table_ptr)) goto page_table_error;
    table_ptr = strip_present_bits(table_ptr);

    //if the page is present, free the physical page
    if((table_ptr[p1_index] & 0b11) != 0){
        void *physicalPage = (void*)(strip_present_bits((uint64_t*)table_ptr[p1_index]));
        MMU_pf_free(physicalPage);
        table_ptr[p1_index] = 0;
    }
    //if set for on demand, then only need to
    else if((table_ptr[p1_index] & 0b1000000000) != 0){
        table_ptr[p1_index] = 0;
    }
    else {
        page_table_error:
        printk_err("attempted to free non virtually allocated page %lx\n", vpage);
    }

}

void MMU_free_pages(void *first_address, int num){
    int i;
    for(i = 0; i < num; i++){
        MMU_free_page(first_address + (i * 4096));
    }
}

/***** heap allocation *****/
KmallocPool pool32 =   {32,   0, (FreeList*)0};
KmallocPool pool64 =   {64,   0, (FreeList*)0};
KmallocPool pool128 =  {128,  0, (FreeList*)0};
KmallocPool pool512 =  {512,  0, (FreeList*)0};
KmallocPool pool1024 = {1024, 0, (FreeList*)0};
KmallocPool pool2048 = {2048, 0, (FreeList*)0};

static void pushBlockToPoolStack(KmallocPool *pool, FreeList *block){
    //LIFO linked list
    block->next = pool->head;
    pool->head = block;

    //increment number of available blocks in the pool
    pool->avail += 1;
}

/**
  * NOTE: todo determine whether or not the blocks in the pool need to be in contiguous pages of virtual memory
  * NOTE: todo figure out if laying the structures on top of the virtual pages makes sense (it forces the pages to become physical)
  * @return the number of blocks added to the pool
  */
static int addBlocksToPool(KmallocPool* pool, int numPagesToChopUp){
    void *pageForBlocks, *curBlockPos;
    int i, counter = 0, numBlocksPerPage = 4096 / pool->max_size;

    pageForBlocks = MMU_alloc_page();
    if(pageForBlocks == 0){
        return 0;
    }
    // pageForBlocks = MMU_alloc_pages(numPagesToChopUp);

    for(i = 0; i < numBlocksPerPage * numPagesToChopUp; i++){
        //grab current position in pageForBlocks
        curBlockPos = pageForBlocks + (pool->max_size * i);

        //overlay a FreeList struct on top of the current block inside the page
        FreeList *newBlock = (FreeList*)curBlockPos;

        pushBlockToPoolStack(pool, newBlock);
        counter += 1;
    }

    return counter;
}

static FreeList *popBlockFromPool(KmallocPool *pool){
    FreeList *toReturn;

    //if we are out of blocks in the pool, add 1 page's worth more
    if(pool->avail == 0){
        int blocksAdded = addBlocksToPool(pool, 1);
        if(!blocksAdded){
            printk_err("out of memory, can't add more blocks to pool\n");
            return (FreeList*)0;
        }
    }

    toReturn = pool->head;
    pool->head = toReturn->next;
    pool->avail -= 1;

    return toReturn;
}


//fills all of the pools with 1 page worth of blocks
void init_kheap(){
    //fill 32 byte pool (should have 128 blocks)
    addBlocksToPool(&pool32, 1);

    //fill 64 byte pool (should have 64 blocks)
    addBlocksToPool(&pool64, 1);

    //fill 128 byte pool (should have 32 blocks)
    addBlocksToPool(&pool128, 1);

    //fill 512 byte pool (should have 8 blocks)
    addBlocksToPool(&pool512, 1);

    //fill 1024 byte pool (should have 4 blocks)
    addBlocksToPool(&pool1024, 1);

    //fill 2048 byte pool (should have 2 blocks)
    addBlocksToPool(&pool2048, 1);

    printk("p32 has %d blocks, p64 has %d blocks, p128 has %d blocks\n", pool32.avail, pool64.avail, pool128.avail);
    printk("p512 has %d blocks, p1024 has %d blocks, p2048 has %d blocks\n", pool512.avail, pool1024.avail, pool2048.avail);

}

void kfree(void *addr){
    KmallocExtra *extra = (KmallocExtra*)(addr - sizeof(KmallocExtra));
    KmallocPool *p = extra->pool;

    if(p == &pool32 || p == &pool64 || p == &pool128 || p == &pool512 || p == &pool1024 || p == &pool2048){
        pushBlockToPoolStack(p, (FreeList*)extra);
        return;
    }

    printk_err("attempted to free memory that didn't even seem to be allocated, fuck you\n");
}

//NOTE todo - determine whether or not the size inside of the extra is the size of the block or the size that was requested to be allocated
//              I am fairly confident it is the second
//NOTE todo - determine whether the blocks should be their indicated size (i.e 32) or their actual size (i.e. 48)
//make sure allocated memory is 16 byte aligned
void *kmalloc(size_t size){
    size_t size_w_extra = size + sizeof(KmallocExtra);
    FreeList *allocatedBlock;
    KmallocPool *fittingPool;
    KmallocExtra *extra;

    if(size == 0){
        return (void*)0;
    }

    if(size_w_extra <= 32){
        fittingPool = &pool32;
    }
    else if(size_w_extra <= 64){
        fittingPool = &pool64;
    }
    else if(size_w_extra <= 128){
        fittingPool = &pool128;
    }
    else if(size_w_extra <= 512){
        fittingPool = &pool512;
    }
    else if(size_w_extra <= 1024){
        fittingPool = &pool1024;
    }
    else if(size_w_extra <= 2048){
        fittingPool = &pool2048;
    }
    else {
        printk_err("tried to allocate too big of a chunk of memory, max is 2032\n");
        return (void *)0;
    }

    //if we made it here, the fitting pool should be set
    allocatedBlock = popBlockFromPool(fittingPool);
    if(!allocatedBlock){
        return 0;
    }
    extra = (KmallocExtra*) allocatedBlock;
    extra->pool = fittingPool;
    extra->size = size;

    //return a pointer to the inside of the block past the extra
    return ((void*)allocatedBlock) + sizeof(KmallocExtra);
}
