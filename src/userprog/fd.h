
#ifndef USERPROG_FD_H
#define USERPROG_FD_H

#include <hash.h>

#define MAX_FD 128

// fd table per process
struct fd_table_t{
    struct list fd_list; 
    bool fd_used_list[MAX_FD];
};

struct fd_t{
    struct list_elem list_elem;
    uint32_t fd;
    struct file* file;
};


void fd_init (struct fd_table_t* fd_table);
struct file* get_open_file(struct fd_table_t* fd_table, uint32_t fd);
bool open_file(struct fd_table_t* fd_table, char* filename, uint32_t* fd_out);
bool close_file(struct fd_table_t* fd_table, uint32_t fd);
void close_all_file(struct fd_table_t* fd_table);
#endif /* userprog/fd.h */
