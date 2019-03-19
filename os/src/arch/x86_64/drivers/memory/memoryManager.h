#ifndef MEM_MANAGER
#define MEM_MANAGER

#include <stdint-gcc.h>
#include <stddef.h>
typedef struct {
    uint64_t beg_addr;
    uint64_t end_addr;
    uint64_t length;
    uint64_t position_inside_segment;
} usable_memory_segment;

typedef struct {
    uint64_t beg_addr;
    uint64_t end_addr;
} blocked_memory_segment;

struct frame_list_node {
    uint8_t used; //1 if used otherwise 0
    uint64_t beg_addr;
    uint64_t end_addr;

    struct frame_list_node *next_frame;
    struct frame_list_node *prev_frame;
};

typedef struct frame_list_node frame_list_node;

typedef struct {
    uint64_t beg_addr;
    uint64_t end_addr;
    uint64_t current_position_in_frame;
} current_node_holder_frame;

typedef struct {
    usable_memory_segment segments[20];
    blocked_memory_segment blocked_segments[20];
    uint32_t seg_length;
    uint32_t bseg_length;
    int curSegment;

    frame_list_node *free_frames_list;
    frame_list_node *used_frames_list;
    current_node_holder_frame node_frame;

    uint64_t end_of_memory;
} memory_info;


void init_usable_segment_struct();
void update_end_of_memory(uint64_t new_end);
void add_segment(uint64_t beg, uint64_t end, uint64_t len);
void add_blocked_segment(uint64_t beg, uint64_t end);

void *MMU_pf_alloc();
void MMU_pf_free(void *pf);
void zero_out_page(void *pf);

void *init_page_table();
void *MMU_alloc_page(uint32_t *phys_addr);
void *MMU_alloc_pages(int, uint32_t *phys_addrs);
void *MMU_alloc_user_page();
void *MMU_alloc_user_pages(int);

void MMU_free_page(void *vpage);
void MMU_free_pages(void *first_address, int num);

void init_kheap();

void *kmalloc(size_t);
void kfree(void*);

void *alloc_dma_coherent(size_t size, uint32_t *phys_addr);

struct FreeList {
    struct FreeList *next;
};

typedef struct FreeList FreeList;

typedef struct {
    int max_size;
    int avail;
    struct FreeList *head;
} KmallocPool;

typedef struct {
    KmallocPool *pool;
    size_t size;
} KmallocExtra;

#endif
