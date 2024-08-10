#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <list.h>

uint8_t * get_new_frame(void);

typedef struct frame_table_t {
    struct list frame_entry_list;
} frame_table_t;

/* Implement second-change page replacement algorithm */
typedef struct frame_entry_t {
    bool protected; // Force not to be evicted
    // bool second_chance;
    struct list_elem list_elem;
    uint8_t *kpage; // Kernel virtual address, updated by get_new_frame function
    uint8_t *upage; // User virtual address, updated by caller of get_new_frame 
    struct thread* owner; // updated by caller of get_new_frame 
} frame_entry_t;

void evict_frame(frame_entry_t* frame_to_evict);
void frame_init(void);

#endif /* vm/frame.h */
