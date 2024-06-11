#include "userprog/fd.h"
#include "filesys/filesys.h"
#include <debug.h>

static bool _next_free_fd(struct fd_table_t* fd_table, uint32_t* fd_out);
// /* Returns a hash value for fd p. */
// static uint32_t fd_hash (const struct hash_elem *p_, void *aux UNUSED)
// {
//   const struct fd_t *p = hash_entry (p_, struct fd_t, hash_elem);
//   return hash_int (p->fd);
// }

// /* Returns true if fd a precedes fd b. */
// static bool fd_less (const struct hash_elem *a_, const struct hash_elem *b_,
//            void *aux UNUSED)
// {
//   const struct fd_t *a = hash_entry (a_, struct fd_t, hash_elem);
//   const struct fd_t *b = hash_entry (b_, struct fd_t, hash_elem);

//   return a->fd < b->fd;
// }

// TODO: Sync
static bool _next_free_fd(struct fd_table_t* fd_table, uint32_t* fd_out){
    for(int i = 0; i < MAX_FD; ++i){
        if(!fd_table->fd_used_list[i]){
            *fd_out = i;
            fd_table->fd_used_list[i] = true;
            return true;
        }
    }
    return false;
}
// hash_init(&(fd_table->fd_table), fd_hash, fd_less, NULL);

void fd_init(struct fd_table_t* fd_table){
    list_init(&(fd_table->fd_list));
    
    fd_table->fd_used_list[0] = true;
    fd_table->fd_used_list[1] = true;
    fd_table->fd_used_list[2] = true;
}

struct file* get_open_file(struct fd_table_t* fd_table, uint32_t fd){
    struct list_elem *e;
    struct file* file = NULL;
    for (e = list_begin (&(fd_table->fd_list)); e != list_end (&(fd_table->fd_list));
        e = list_next (e)){
        struct fd_t *fd_t = list_entry (e, struct fd_t, list_elem);
        if(fd_t->fd == fd){
            file = fd_t->file; 
        }
    }
    return file;
}


bool open_file(struct fd_table_t* fd_table, char* filename, uint32_t* fd_out){
    if(!(_next_free_fd(fd_table, fd_out))){
        return false;
    }
    struct file* file = filesys_open(filename);
    if(!file){
        return false;
    }
    struct fd_t fd;
    fd.fd = fd_out;
    fd.file = file;
    list_push_back(&(fd_table->fd_list), &fd.list_elem);
    return true;
}
