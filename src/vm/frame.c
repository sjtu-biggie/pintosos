#include "vm/frame.h"

#include <string.h>
#include "threads/malloc.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "threads/palloc.h"
#include "vm/page.h"
#include <hash.h>
#include <debug.h>

/* Storing all the frame_entry across the kernel */
static frame_table_t frame_table;

frame_entry_t* find_eviction_frame(void);

frame_entry_t* find_eviction_frame(void){
    struct list_elem *e = list_begin (&frame_table.frame_entry_list);
    while(e != list_end (&frame_table.frame_entry_list)) 
    {
		frame_entry_t *frame_entry = list_entry(e, struct frame_entry_t, list_elem);
        /* Second-change algorithm */
        /*TODO: ALIAS*/
        if(pagedir_is_accessed(frame_entry->owner->pagedir, frame_entry->upage)){
            pagedir_set_accessed(frame_entry->owner->pagedir, frame_entry->upage, false); //TODO
            list_push_back(&frame_table.frame_entry_list, e);
            struct list_elem *next = list_next (e); 
            list_remove(e);
            e = next;
        }else if(frame_entry->protected){
            e = list_next (e); 
        }else{
            return frame_entry;
        }
    }
    return NULL;
}

uint8_t * get_new_frame(void){
    uint8_t *kpage = palloc_get_page (PAL_USER); // Pointer pointing to the page
    int try_count = 0;
    while(kpage == NULL){
        frame_entry_t* frame_to_evict = find_eviction_frame();
        ASSERT (frame_to_evict); // Failure here indicate 100% full of memory and swap
        evict_frame(frame_to_evict);
        kpage = palloc_get_page (PAL_USER);
        try_count += 1;
        ASSERT(try_count < 10);
    }
    // debug_printf("");
    frame_entry_t* entry = (frame_entry_t*)malloc(sizeof(frame_entry_t));
    entry->kpage = kpage;
    list_push_back(&(frame_table.frame_entry_list), &(entry->list_elem));
    return kpage;
}


void evict_frame(frame_entry_t* frame_to_evict){
    ASSERT(0); // TODO: Reach here no 
    list_remove(&frame_to_evict->list_elem);
    /*TODO: ALIAS*/
    if(pagedir_is_dirty(frame_to_evict->owner->pagedir, frame_to_evict->upage)){
        // TODO: Write to file system or swap
        page_entry_t* page_entry = page_table_lookup(&frame_to_evict->owner->page_table, frame_to_evict->upage);
        if(page_entry->source == FROM_FILE){

        }else if(page_entry->source == FROM_SWAP){

        }else{
            ASSERT(0);
        }
    }
    palloc_free_page(frame_to_evict->kpage);
    free(frame_to_evict);
}

void frame_init(void){
    list_init(&frame_table.frame_entry_list);
}