/*
 * Copyright 2015 Davide Kirchner
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * Defines a restricted task-like API for the special IDLE task
 */

#ifndef __IDLE_H__
#define __IDLE_H__

#include <time.h>
#include <sched.h>
#include <pthread.h>
#include <semaphore.h>

#include "common.h"
#include "resources.h"

struct taskset;  /* can't include taskset before defining idle_task */


#ifndef IDLE_TASK_PRIORITY
#define IDLE_TASK_PRIORITY 1
#endif


/**
 * Parameters required to start and run a periodic task
 */
struct idle_task {
  /* To be set before starting the task (or by task_init_str) */
  struct taskset *ts;   /* pointer to the taskset containing some shared vars */

  /* Set at creation/initialization time */
  pthread_t tid;        /* the thread id */

  /* Updated and used during execution */
  unsigned long last_tick;      /* tick value last time this task executed */
  bool quit;            /* when true, instructs the task to stop gracefully */
  bool done;            /* becomes true after the task has stopped gracefully */
};

#include "taskset.h"  /* Deferred include avoids circular dependency */


void idle_task_init(struct idle_task *it);

void idle_task_create(struct idle_task *it);

void idle_task_join(struct idle_task *it);

#endif
