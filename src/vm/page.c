#include "vm/page.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <list.h>
#include "threads/pte.h"
#include "threads/malloc.h"

static uint32_t pt_hash (const struct hash_elem *p_, void *aux UNUSED)
{
  const struct page_entry_t *p = hash_entry (p_, struct page_entry_t, page_hash);
  return hash_bytes (&p->upage, sizeof p->upage);
}

/* Returns true if fd a precedes fd b. */
static bool pt_less (const struct hash_elem *a_, const struct hash_elem *b_,
           void *aux UNUSED)
{
  const struct page_entry_t *a = hash_entry (a_, struct page_entry_t, page_hash);
  const struct page_entry_t *b = hash_entry (b_, struct page_entry_t, page_hash);
  return a->upage < b->upage;
}

page_entry_t* page_table_lookup(page_table_t* page_table, uint8_t* upage){
  struct page_entry_t page_entry;
  struct hash_elem *e;
  page_entry.upage = upage;
  e = hash_find(&page_table->hash_table, &page_entry.page_hash);
  return e != NULL ? hash_entry(e, struct page_entry_t, page_hash) : NULL;
}

page_entry_t* page_table_new_entry(page_table_t* page_table){
  page_entry_t* entry = (page_entry_t*)malloc(sizeof(page_entry_t));
  hash_insert(&page_table->hash_table, &entry->page_hash);
  return entry;
}

void page_table_init(page_table_t* page_table){
    hash_init(&page_table->hash_table, pt_hash, pt_less, NULL);
}