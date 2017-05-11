#ifndef MEM_MANAGER
#define MEM_MANAGER

#include <stdint-gcc.h>

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
} memory_info;


void init_usable_segment_struct(memory_info*);
void add_segment(memory_info*, uint64_t beg, uint64_t end, uint64_t len);
void add_blocked_segment(memory_info*, uint64_t beg, uint64_t end);

void *MMU_pf_alloc(memory_info*);
void MMU_pf_free(memory_info*,void *pf);

#endif
