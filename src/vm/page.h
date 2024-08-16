#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include "threads/pte.h"
#include "filesys/file.h"

enum source{
    SOURCE_EXECUTABLE,
    SOURCE_MMAP,
    SOURCE_VOID // No longer need to write back!
};

typedef struct file_info_t {
    struct file* file;
    off_t ofs;
    int size;
} file_info_t;

typedef struct page_table_t {
    struct hash hash_table;
} page_table_t;

typedef struct page_entry_t {
    struct hash_elem page_hash;
    enum source source;
    uint8_t* upage; //user virtual address
    file_info_t file_info;
    int swap_id;
} page_entry_t;

void page_table_init(page_table_t* page_table);

/* Return the page_entry giving page table and virtual address*/
page_entry_t* page_table_lookup(page_table_t* page_table, uint8_t* upage);

page_entry_t* page_table_new_entry(page_table_t* page_table);

#endif /* vm/page.h */
