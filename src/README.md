# Project 1

# Project 2
## Design of Argument Passing
We do the argument passing in `process.c::start_process`


First, break down the command string into arguments. Because the order we put argument on the stack is **reverse** to the order we parse it, create a `saved_param` to temporaily store them.

Final order:

1. argv value
2. word-align
3. 0
4. argv position
5. argc
6. return address

Notice that the `palloc_free_page` will free the resource, this will cause the parsing failure.

Now we can check the validity of our solution using `hex_dump`. But we can't pass any test case becuase they rely on `WRITE` system call to write to the console output.

## Design of File Handle
This is actually implementing the per-process `fd_table`, shared by all threads. The data structure is:
```cpp
struct fd_table_t{
    struct list fd_list; // List of open file handlers
    bool fd_used_list[MAX_FD]; 
};

struct fd_t{
    struct list_elem list_elem;
    uint32_t fd;
    struct file* file; // Open file abstraction is here!
};
``` 
Notice that the read/write offset inside the file abstraction, rather than fd table.

The READ and WRITE system calls are straightforward after implementing a correct fd system.

In **thread.c:**
```cpp
struct thread
{
    struct fd_table_t fd_table;
}
```
We have no lock here because pintos is single-threaded

## Design of Exec & Wait
This part is the most demanding one. 
First we post the data structure of exec block here
```cpp
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

```
We have a global(kernel-wide) linked list that is a list of `exec_block_t`. For every existing exec-pair, there will be a exec_block instance. exec_block also control the communication between parents and child. 

In thread.c, the global structure:
```c++
static struct list exec_list;
static struct lock exec_list_lock;
```

After having a basic sense of our data type, lets start from a normal exec workflow. 

First, we'll create the exec block. We only initialize a few field, and push it back onto the exec list. 
```cpp
struct exec_block_t* thread_create_exec_block(tid_t parent_tid, bool initial){
    //...
    exec_block->status = UNINITIALIZED;
    //...
}

struct exec_block_t* exec_block = thread_create_exec_block(current_thread->tid, false);

```
Then we go to the `process_execute`, in which we run `start_process` in newly created thread. If thread creation failed, parent will get a `TID_ERROR` immediately. Otherwise parent will start to wait for load status.

In the new thread we then call `load`, and we need to tell the child immediately about whether load succeed. Notice here we only `sema_up` to parents when the thread is not the initial thread.

If everything good then the execute call is done.
```cpp
// Parents
case SYS_EXEC:{
    //...
    tid_t tid = process_execute(cmd_line);
    if(tid != TID_ERROR){
        sema_down(&exec_block->exec_sem);
        tid = exec_block->status == THREAD_KILLED ? TID_ERROR : tid;
    }
    //...
}

// Child
static void start_process (void *file_name_){
    //...
    exec_block->status = success ? LOAD_SUCCESS : block->status;
    if(!exec_block->initial){
        sema_up(&block->exec_sem);
    }
    //...
}
```
Then we need to deal with wait, exit and a lot of rabbit holes here. 
### Children die before parents, parents call wait
This is the normal case. When children reach `thread_exit`, the block->status won't be `PARENT_DIED`, so we will call `sema_up` to notify parents. Whether parents are in "wait" or not, by the time they called wait the semaphore is non-blocking.
### Children die before parents, parents never call wait
The first half of this case is idential to the previous one. But since parents never call wait, we have to handle the cleanup on parent's `thread_exit`. That's how `thread_clear_exec_block_as_parent` come into use.
### Parents die before children 
In the `thread_clear_exec_block_as_parent`, we traverse all the exec blocks, and if there are children alive, the block will be marked as `PARENT_DIED`
### Parents call wait twice / illegal children id
This is handled in `process_wait`. Basically the `thread_get_exec_block_from_child` function can process all possible scenarios.


```cpp
  if(block){
    if(block->status != THREAD_EXIT){
        block->status = THREAD_KILLED;
        thread_current()->exit_status = -1;
    }
    block->exit_status = thread_current()->exit_status;
    printf("%s: exit(%d)\n", block->command, block->exit_status);
    //...
    if(block->status == PARENT_DIED){
      list_remove(&block->list_elem);
      if(block->command){
        free(block->command);
        block->command = NULL;
      }
      free(block);
    }else{
      sema_up(&block->exec_sem);
    } 
  }
  thread_clear_exec_block_as_parent(thread_current()->tid);

```


### Remove exec block
There are four scenarios when exec blocks need to be cleaned.
## Misc
1. CRLF issue. Developing in Windows OS + vscode + docker + git, each tool has its way of processing new line character. When the file read has more size than it should be in the test, take this into consideration.
2. Redirection cause line repetition. Before implementing all features, I found that some tests are good if ran manually, but failed in test script after they are redirected into a file for check. This is somehow fixed by finishing the lab2.



