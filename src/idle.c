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
 * Implementation of the idle task, as described in "idle.h"
 */

#include <errno.h>
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>

#include "common.h"
#include "idle.h"

/* from task.c */
void tick_pp(struct taskset *ts, int id, int res, int type,
    unsigned long *last_tick);


static void idle_body(struct idle_task *it) {
  int s;
  struct timespec sleeptime;

  s = pthread_setname_np(pthread_self(), "idle");
  if (s) {
    printf_log_perror(LOG_WARNING, s,
        "Task activation failed: pthread_setname_np returned error: ");
    return;
  }

  sleeptime.tv_sec = 0;
  sleeptime.tv_nsec = 1;  /* Will be rounded _up_ to resolution */

  printf_log(LOG_INFO, "Idle task started!\n");

  while (! it->quit) {
    run_assert(0 == sem_wait(&it->ts->task_lock));
    tick_pp(it->ts, -1, 0, EVT_RUN, &it->last_tick);
    run_assert(0 == sem_post(&it->ts->task_lock));

    if (options.idle_yield)
      run_assert(0 == pthread_yield());
    if (options.idle_sleep)
      run_assert(0 == clock_nanosleep(CLOCK_MONOTONIC, 0, &sleeptime, NULL));
  }

  it->done = true;
}


static void *idle_function(void* it) {
  idle_body((struct idle_task*) it);
  return NULL;
}


/* documented in header file */
void idle_task_init(struct idle_task* task) {
  task->ts = NULL;
  task->last_tick = 0UL;
  task->quit = false;
  task->done = false;
}



/* handle errors that may happen in task_create */
#define handle_error(en, fname) \
  do { \
    printf_log_perror(LOG_WARNING, en, \
        "Couldn't start idle task: Got an error while calling function %s: ", \
        fname); \
    return; \
  } while (0)

#define handle_error_clean(en, fname) \
  do { pthread_attr_destroy(&tattr); handle_error(en, fname); \
  } while (0)


/* documented in header file */
void idle_task_create(struct idle_task *it) {
  pthread_attr_t tattr;                 /* thread attributes */
  struct sched_param sched_param;       /* scheduling parameters */
  int s;                        /* return value of called library functions */

  printf_log(LOG_DEBUG, "Starting creation of idle task.\n");

  s = pthread_attr_init(&tattr);
  if (s) handle_error(s, "pthread_attr_init");

  s = pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_JOINABLE);
  if (s) handle_error_clean(s, "pthread_attr_setdetachstate");

  if (options.idle_rt_sched) {
    s = pthread_attr_setinheritsched(&tattr, PTHREAD_EXPLICIT_SCHED);
    if (s) handle_error_clean(s, "pthread_attr_setinheritsched");

    s = pthread_attr_setschedpolicy(&tattr, TASK_SCHED_POLICY);
    if (s) handle_error_clean(s, "pthread_attr_setschedpolicy");

    sched_param.sched_priority = IDLE_TASK_PRIORITY;
    s = pthread_attr_setschedparam(&tattr, &sched_param);
    if (s) handle_error_clean(s, "pthread_attr_setschedparam");
  }
  
  if (options.with_affinity) {
    assert(CPU_COUNT(&options.task_cpuset) == 1);
    s = pthread_attr_setaffinity_np(
        &tattr, sizeof(cpu_set_t), &options.task_cpuset);
    if (s) handle_error_clean(s, "pthread_attr_setaffinity_np");
  }

  s = pthread_create(&it->tid, &tattr, idle_function, it);
  if (s) handle_error_clean(s, "pthread_create");

  pthread_attr_destroy(&tattr);
}
#undef handle_error
#undef handle_error_clean



/* documented in header file */
void idle_task_join(struct idle_task *it) {
  int s;

  s = pthread_join(it->tid, NULL);
  if (s) printf_log_perror(LOG_WARNING, s,
      "Error calling pthread_join for <%s>: ", "idle");
}
