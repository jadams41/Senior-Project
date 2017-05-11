#include "memoryManager.h"
#include <stdint-gcc.h>
#include "printk.h"

void init_usable_segment_struct(memory_info *new_struct){
    int i;
    for(i = 0; i < 20; i++){
        usable_memory_segment *seg = &(new_struct->segments[i]);
        seg->beg_addr = 0;
        seg->end_addr = 0;
        seg->length = 0;
        seg->position_inside_segment = 0;

        blocked_memory_segment *bseg = &(new_struct->blocked_segments[i]);
        bseg->beg_addr = -1;
        bseg->end_addr = -1;
    }
    new_struct->seg_length = 0;
    new_struct->bseg_length = 0;
    new_struct->curSegment = -1;

    new_struct->node_frame.beg_addr = 0;
    new_struct->node_frame.end_addr = 0;
    new_struct->node_frame.current_position_in_frame = 0;

    new_struct->free_frames_list = 0;
    new_struct->used_frames_list = 0;
}

//todo, check if segment is smaller than 4K and if so don't add
void add_segment(memory_info *info, uint64_t beg, uint64_t end, uint64_t len){
    if(info->seg_length == 19){
        printk("[ERR]: segment array out of space\n");
        return;
    }

    if(beg == 0){
        printk("[ERR]: received a segment beginning at 0, probably invalid\n");
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

void add_blocked_segment(memory_info *info, uint64_t beg, uint64_t end){
    if(info->bseg_length == 19){
        printk("[ERR]: blocked segment array out of space\n");
        return;
    }

    uint32_t length = info->bseg_length;
    info->blocked_segments[length].beg_addr = beg;
    info->blocked_segments[length].end_addr = end;

    info->bseg_length += 1;
}

//return end address of blocked_segment if the passed in address is in any of the blocked regions, otherwise 0
static uint64_t address_in_blocked_memory(memory_info *info, uint64_t addr){
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
static int grab_next_frame(memory_info *info){
    int end_of_blocked_memory;
    if(info->curSegment == -1 || info->curSegment == (int)info->seg_length){
        //out of memory
        return 0;
    }
    usable_memory_segment *curSeg = info->segments + info->curSegment;
    while(1){
        int possible_next_frame_addr = curSeg->position_inside_segment;

        //check if there is a 4k frame inside of the currentSegment
        if((possible_next_frame_addr + 4096) > curSeg->end_addr){
            //go to the next memory segments
            if(info->curSegment == -1 || info->curSegment == (int)info->seg_length){
                //out of memory
                return 0;
            }
            curSeg = info->segments + (++(info->curSegment));
        }
        else if((end_of_blocked_memory = address_in_blocked_memory(info, possible_next_frame_addr)) || address_in_blocked_memory(info, possible_next_frame_addr + 4096)){
            if(address_in_blocked_memory(info, possible_next_frame_addr + 4096)){
                end_of_blocked_memory = address_in_blocked_memory(info, possible_next_frame_addr + 4096);
            }
            if(end_of_blocked_memory < (curSeg->end_addr + 1)){
                curSeg->position_inside_segment = end_of_blocked_memory + 1;
            }
            else {
                curSeg = info->segments + (++(info->curSegment));
            }
        }
        else {
            curSeg->position_inside_segment += 4096;
            return possible_next_frame_addr;
        }
    }
    return 0;
}

//this method should be called when the pf linked list is empty
//NOTE: this will happen when the first frame is requested
//NOTE: this adds 20 page frames to the linked list at a time
//returns 1 if out of memory (or no segments have been initialized yet)
static int populate_pf_list(memory_info *info){
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

//will attempt to grab a page frame off of the page frame list
//then will add a
void *MMU_pf_alloc(memory_info *info){
    int err;
    if(info->free_frames_list == 0){
        err = populate_pf_list(info);
        if(err){
            printk("There was an error populating the page frame list\n");
        }
    }

    frame_list_node *toAlloc = info->free_frames_list;
    if(toAlloc == 0){
        printk("[ERR]: there was an error allocating a frame\n");
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

    return (void*)toAlloc->beg_addr;
}

void MMU_pf_free(memory_info *info, void *pf){
    frame_list_node *curUsedNode = info->used_frames_list;
    while(1){
        if(curUsedNode == 0){
            printk("[ERR]: tried to free a node that wasn't allocated\n");
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
