#include "userprog/syscall.h"
#include "userprog/pagedir.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <debug.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

#include "userprog/fd.h"
#include "userprog/process.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "devices/shutdown.h"
#include "devices/input.h"



#define GET_ARGUMENT(ptr, index, type) is_valid((void*)((uint32_t)(ptr) + 4 * index + 3))?*(type*)((uint32_t)(ptr) + 4 * index) : *(type*)thread_exit_with_status(-1)
#define READ_ERROR -1
#define READ_SUCCESS 0

#define CHUNK 128

static void syscall_handler (struct intr_frame *);
static bool is_valid(const void* vaddr);
static void* thread_exit_with_status(int status);


void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

// Determine if a user pointer is legal
// Kill if passed illegal address
bool is_valid(const void* vaddr){
  if(!is_user_vaddr(vaddr)){
    return false;
  }
  return pagedir_get_page(active_pd(), vaddr) != NULL;
}

static void* thread_exit_with_status(int status){
    // Normally Exit
    struct exec_block_t* exec_block = thread_get_exec_block_from_child(thread_current()->tid);
    if(exec_block){
        exec_block->status = THREAD_EXIT;
    }

    thread_current()->exit_status = status;
    thread_exit();

    NOT_REACHED();
    return NULL;
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  // intr_dump_frame(f);
  if(!is_valid(f->esp)){
    debug_printf("ERROR: invalid system call frame pointer %p\n", f->esp);
    thread_exit_with_status(-1);
  }

  struct thread* current_thread = thread_current();
  struct fd_table_t* fd_table = &(current_thread->fd_table);

  uint32_t syscall_number = GET_ARGUMENT(f->esp, 0, uint32_t);
  debug_printf("Handling system call %d\n", syscall_number);
  switch (syscall_number)
  {
  case SYS_HALT:{
    shutdown_power_off();
    break;
  }
  case SYS_EXIT:{
    int status = GET_ARGUMENT(f->esp, 1, int);
    thread_exit_with_status(status);
    break;
  }
  case SYS_EXEC:{
    const char* cmd_line = GET_ARGUMENT(f->esp, 1, char*);
    if(!is_valid(cmd_line)){
      debug_printf("ERROR: exec failed due to invalid filename %p\n", cmd_line);
      thread_exit_with_status(-1);
    }
    // Parent procesws
    tid_t tid = process_execute(cmd_line);
    f->eax = tid;
    if(tid == TID_ERROR){
      return;
    }
    struct exec_block_t* exec_block = thread_create_exec_block(current_thread->tid, false);
    sema_down(&exec_block->exec_sem);

    f->eax = exec_block->status == LOAD_FAILURE ? TID_ERROR : tid;
    break;
  }
  case SYS_WAIT:{
    int pid =  GET_ARGUMENT(f->esp, 1, int);
    int result = process_wait(pid);
    f->eax = result;
    break;
  }
  case SYS_CREATE:{
    const char* filename = GET_ARGUMENT(f->esp, 1, char*);
    unsigned initial_size = GET_ARGUMENT(f->esp, 2, unsigned);
    if(!is_valid(filename) || !filename){
      debug_printf("ERROR: opening failed due to invalid filename %p\n", filename);
      thread_exit_with_status(-1);
    }

    lock_acquire(&filesys_lock);
    if(!filesys_create(filename, initial_size)){
      debug_printf("ERROR: create failed\n");
      f->eax = (uint32_t)false;
    }else{
      f->eax = (uint32_t)true;
    }
    lock_release(&filesys_lock);
    break;
  }
  case SYS_REMOVE:
  {
    const char* filename = GET_ARGUMENT(f->esp, 1, char*);
    if(!is_valid(filename) || !filename){
      debug_printf("ERROR: Removing failed due to invalid filename %p\n", filename);
      thread_exit_with_status(-1);
    }
    
    lock_acquire(&filesys_lock);
    bool success = filesys_remove(filename);
    lock_release(&filesys_lock);

    f->eax = success;
    break;
  }
  case SYS_OPEN:{
    char* filename = GET_ARGUMENT(f->esp, 1, char*);
    if(!is_valid(filename) || !filename){
      debug_printf("ERROR: opening failed due to invalid filename %p\n", filename);
      thread_exit_with_status(-1);
    }

    uint32_t fd_out;
    lock_acquire(&filesys_lock);
    bool success = open_file(fd_table, filename, &fd_out);
    lock_release(&filesys_lock);
    if(!success){
      debug_printf("ERROR: failed to open file%p\n", filename);
      f->eax = -1;
    }else{
      f->eax = fd_out;
    }
    break;
  }
  case SYS_FILESIZE:{
    const uint32_t fd = GET_ARGUMENT(f->esp, 1, uint32_t);
    struct file* file = get_open_file(fd_table, fd);
    if(!file){
      debug_printf("ERROR: failed to get file from fd %d\n", fd);
      thread_exit_with_status(-1);
    }

    lock_acquire(&filesys_lock);
    debug_printf("size:%d\n", file_length(file));
    f->eax = file_length(file);
    lock_release(&filesys_lock);
    break;
  }

  case SYS_READ:{
    uint32_t fd = GET_ARGUMENT(f->esp, 1, uint32_t);
    char* buffer = GET_ARGUMENT(f->esp, 2, char*);
    unsigned size = GET_ARGUMENT(f->esp, 3, unsigned);
    uint32_t size_read = 0;
    if(!is_valid(buffer)){
      debug_printf("ERROR: writing failed due to invalid user buffer %p\n", buffer);
      f->eax = READ_ERROR;
      return;
    }
    if(fd == STDIN_FILENO){
      uint8_t key = input_getc();
      buffer[0] = key;
      size_read = 1;
    }else{
      struct file* file = get_open_file(fd_table, fd);
      if(!file){
        // Failed to get open file
        debug_printf("ERROR: failed to get open file for fd %d\n", fd);
        f->eax = READ_ERROR;
        return;
      }
      lock_acquire(&filesys_lock);
      size_read = file_read(file, buffer, size);
      lock_release(&filesys_lock);
    }
    // debug_printf("expect size: %d, size_read %d\n", size, size_read);
    f->eax = size_read;
    break;
  }
  case SYS_WRITE:{
    uint32_t fd = GET_ARGUMENT(f->esp, 1, uint32_t);
    const char* buffer = GET_ARGUMENT(f->esp, 2, char*);
    unsigned size = GET_ARGUMENT(f->esp, 3, unsigned);
    if(size == 0) return;

    uint32_t size_written = 0;
    if(!is_valid(buffer)){
      debug_printf("ERROR: writing failed due to invalid user buffer %p\n", buffer);
      thread_exit_with_status(-1);
    }
    if(fd == STDOUT_FILENO){
      // printf("size: %d\n", size);
      int iteration = (size - 1) / CHUNK;
      for(int i = 0; i <= iteration; ++i){
        putbuf(buffer + iteration * CHUNK,  size);
      }
      size_written = size;
    }else{
      struct file* file = get_open_file(fd_table, fd);
      if(!file){
        // Failed to get open file
        debug_printf("ERROR: failed to get open file for fd %d\n", fd);
        thread_exit_with_status(-1);
      }
      lock_acquire(&filesys_lock);
      size_written = file_write(file, buffer, size);
      lock_release(&filesys_lock);
    }      

    f->eax = size_written;
    // debug_printf("expect size: %d, size_write %d\n", size, size_written);
    break;
  }
  case SYS_SEEK:{
    uint32_t fd = GET_ARGUMENT(f->esp, 1, uint32_t);
    unsigned position = GET_ARGUMENT(f->esp, 2, unsigned);
    struct file* file = get_open_file(fd_table, fd);
    if(!file){
      // Failed to get open file
      debug_printf("ERROR: failed to get open file for fd %d\n", fd);
      thread_exit_with_status(-1);
    }
    lock_acquire(&filesys_lock);
    file_seek(file, position);
    lock_release(&filesys_lock);
    break;
  }
  case SYS_TELL:{
    uint32_t fd = GET_ARGUMENT(f->esp, 1, uint32_t);
    struct file* file = get_open_file(fd_table, fd);
    if(!file){
      // Failed to get open file
      debug_printf("ERROR: failed to get open file for fd %d\n", fd);
      thread_exit_with_status(-1);
    }
    f->eax = file_tell(file);
    break;
  }
  case SYS_CLOSE:{
    uint32_t fd = GET_ARGUMENT(f->esp, 1, uint32_t);

    lock_acquire(&filesys_lock);
    if(!close_file(fd_table, fd)){
      debug_printf("ERROR: closing file failed %d\n", fd);
      thread_exit_with_status(-1);
    }
    lock_release(&filesys_lock);

    break;
  }
  case SYS_MMAP:
  case SYS_MUNMAP:
  case SYS_CHDIR:
  case SYS_MKDIR:
  case SYS_READDIR:
  case SYS_ISDIR:
  case SYS_INUMBER:
  default:
    debug_printf ("ERROR: system call %d not implemented \n", syscall_number );
    thread_exit_with_status(-1);
    break;
  }
}
