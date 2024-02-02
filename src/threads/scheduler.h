#ifndef THREADS_SCHEDULER_H
#define THREADS_SCHEDULER_H

#include <debug.h>
#include <list.h>

typedef enum scheduler_type{
    SCHEDULER_PRIORITY,
    SCHEDULER_ADVANCED,
    SCHEDULER_NAIVE
} scheduler_type ;

void set_scheduler(scheduler_type);
scheduler_type get_scheduler_type();
struct thread* next_thread_to_run(struct list *ready_list);

#endif