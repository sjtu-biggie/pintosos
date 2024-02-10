#include <debug.h>
#include "threads/scheduler.h"
#include "threads/thread.h"

static scheduler_type type;

void set_scheduler(scheduler_type t){
    type = t;
}

scheduler_type get_scheduler_type(void){
    return type;
}

struct thread* next_thread_to_run(struct list *ready_list){

    ASSERT(!list_empty(ready_list));

    switch(type){
        case SCHEDULER_ADVANCED: {
        }
        case SCHEDULER_NAIVE: {
            return list_entry (list_pop_front (ready_list), struct thread, elem);
            break;
        }
        case SCHEDULER_PRIORITY: {
            struct thread* thread_to_operate = NULL;
            int min_priority = PRI_MIN - 1; 
            for (struct list_elem* e = list_begin (ready_list); e != list_end (ready_list); e = list_next (e)){
                struct thread *t = list_entry (e, struct thread, elem);
                int real_priority = thread_get_priority_any(t);
                if( real_priority > min_priority){
                    min_priority = real_priority;
                    thread_to_operate = t;
                }
            }
            ASSERT(thread_to_operate != NULL);
            list_remove(&thread_to_operate->elem);

            return thread_to_operate;
            break;
        }
    }
    ASSERT(0);
    return NULL;
}