#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include <debug.h>
#include <list.h>
#include <stdint.h>
#include "threads/synch.h"
#include "threads/scheduler.h"
#include "threads/fpoint.h"

#ifdef USERPROG
#include "userprog/fd.h"
#endif

enum exec_status
  {
    UNINITIALIZED,        /* load() failure */
    INIT_SUCCESS,        /* load() failure */
    LOAD_SUCCESS,     /* Not running but ready to run. */
    THREAD_EXIT,     /* Not running but ready to run. */
    THREAD_KILLED,     /* Not running but ready to run. */
    PARENT_DIED,
  };


struct exec_block_t{
   int pid;                   /* child tid */
   int ppid;                  /* parent tid */
   int exit_status;           /* child exit status */
   enum exec_status status;    /* exec status */
   struct semaphore exec_sem;   /* exec semaphore */
   struct list_elem list_elem; 
   char* command;
   struct file* executable;
   bool initial; // If the parent is the initial process
};

/* States in a thread's life cycle. */
enum thread_status
  {
    THREAD_RUNNING,     /* Running thread. */
    THREAD_READY,       /* Not running but ready to run. */
    THREAD_BLOCKED,     /* Waiting for an event to trigger. */
    THREAD_DYING        /* About to be destroyed. */
  };

/* Thread identifier type.
   You can redefine this to whatever type you like. */
typedef int tid_t;
#define TID_ERROR ((tid_t) -1)          /* Error value for tid_t. */

/* Thread priorities. */
#define PRI_MIN 0                       /* Lowest priority. */
#define PRI_DEFAULT 31                  /* Default priority. */
#define PRI_MAX 63                      /* Highest priority. */

#define THREAD_NOT_SLEEP -1             /* Thread Sleep State */

#define MAX_NESTED_LEVEL 8
/* A kernel thread or user process.

   Each thread structure is stored in its own 4 kB page.  The
   thread structure itself sits at the very bottom of the page
   (at offset 0).  The rest of the page is reserved for the
   thread's kernel stack, which grows downward from the top of
   the page (at offset 4 kB).  Here's an illustration:

        4 kB +---------------------------------+
             |          kernel stack           |
             |                |                |
             |                |                |
             |                V                |
             |         grows downward          |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             +---------------------------------+
             |              magic              |
             |                :                |
             |                :                |
             |               name              |
             |              status             |
        0 kB +---------------------------------+

   The upshot of this is twofold:

      1. First, `struct thread' must not be allowed to grow too
         big.  If it does, then there will not be enough room for
         the kernel stack.  Our base `struct thread' is only a
         few bytes in size.  It probably should stay well under 1
         kB.

      2. Second, kernel stacks must not be allowed to grow too
         large.  If a stack overflows, it will corrupt the thread
         state.  Thus, kernel functions should not allocate large
         structures or arrays as non-static local variables.  Use
         dynamic allocation with malloc() or palloc_get_page()
         instead.

   The first symptom of either of these problems will probably be
   an assertion failure in thread_current(), which checks that
   the `magic' member of the running thread's `struct thread' is
   set to THREAD_MAGIC.  Stack overflow will normally change this
   value, triggering the assertion. */
/* The `elem' member has a dual purpose.  It can be an element in
   the run queue (thread.c), or it can be an element in a
   semaphore wait list (synch.c).  It can be used these two ways
   only because they are mutually exclusive: only a thread in the
   ready state is on the run queue, whereas only a thread in the
   blocked state is on a semaphore wait list. */
struct donation_block{
   struct thread* donator_thread;
   struct lock* donator_wait_on_lock;
   struct list_elem donation_elem;     /* List element for donation */
};

struct thread
  {
    /* Owned by thread.c. */
    tid_t tid;                          /* Thread identifier. */
    tid_t ppid;
    enum thread_status status;          /* Thread state. */
    char name[16];                      /* Name (for debugging purposes). */
    uint8_t *stack;                     /* Saved stack pointer. */
    int priority;                       /* Priority. */
    struct list_elem allelem;           /* List element for all threads list. */

    /* Shared between thread.c and synch.c. */
    struct list_elem elem;              /* List element. */

    /* Who is waiting on this who*/
    struct list priority_list;

    #ifdef USERPROG
    /* File descriptor */
    struct fd_table_t fd_table;
    #endif

    // Maximum 8 nested level
    struct donation_block donation_blocks[MAX_NESTED_LEVEL];

    struct lock* wait_on_lock;
    struct thread* wait_on_thread; 

    int64_t sleep_time;
    struct semaphore sleep_semaphore;

    int exit_status;

    int nice;
    fixed_point recent_cpu;

#ifdef USERPROG
    /* Owned by userprog/process.c. */
    uint32_t *pagedir;                  /* Page directory. */
#endif

    /* Owned by thread.c. */
    unsigned magic;                     /* Detects stack overflow. */
  };


/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
extern bool thread_mlfqs;

void thread_init (void);
void thread_start (void);

void thread_tick (void);
void thread_print_stats (void);

typedef void thread_func (void *aux);
tid_t thread_create (const char *name, int priority, thread_func *, void *);

void thread_block (void);
void thread_unblock (struct thread *);

struct thread *thread_current (void);
tid_t thread_tid (void);
const char *thread_name (void);

void thread_exit (void) NO_RETURN;
void thread_yield (void);

/* Performs some operation on thread t, given auxiliary data AUX. */
typedef void thread_action_func (struct thread *t, void *aux);
void thread_foreach (thread_action_func *, void *);

bool thread_is_alive(struct thread*);

scheduler_type thread_get_priority_any (struct thread*);
int thread_get_priority (void);
void thread_set_priority (int);

int thread_get_nice (void);
void thread_set_nice (int);
int thread_get_recent_cpu (void);
int thread_get_recent_cpu_any (struct thread*);
int thread_get_load_avg (void);


/* Accept priority donation from another thread represented by list_elem*/
void thread_accept_donation(struct thread*, struct thread*, struct lock*);
void thread_retrieve_donation(struct thread*, struct lock*);

/* Exec and wait */
struct thread* get_thread_by_tid(tid_t tid);
struct exec_block_t* thread_get_exec_block_from_child(tid_t tid);
void thread_exec_block_init(tid_t parent_tid, tid_t child_tid);
struct exec_block_t* thread_create_exec_block(tid_t parent_tid, bool initial);
void thread_clear_exec_block_as_parent(tid_t parent_tid);

/* Debugging */
void print_thread_internal(struct thread*);
void print_thread(char* name);

#endif /* threads/thread.h */
