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

#include <assert.h>
#include <errno.h>
#include <pthread.h>

#include "observer.h"
#include "task.h"
#include "taskset.h"
#include "periodic.h"
#include "common.h"


void observer_ctx_init(struct observer_ctx *ctx) {
  ctx->last_counter = 0;
}

void observer_body(struct taskset *ts,
    unsigned long *obs_it, unsigned long *dmiss) {
  int i;
  struct task_params *task;
  struct observer_ctx *ctx;
  unsigned long diff;

  (*obs_it) ++;

  for (i = 0; i < ts->tasks_count; i++) {
    task = &ts->tasks[i];
    ctx = &ts->observer_ctxs[i];

    if (options.with_global_lock)
      run_assert(0 == sem_wait(&options.global_lock));

    if (! task->activated) {
      /* printf_log(LOG_DEBUG, "'%s' not active yet.\n", task->name); */
    }
    else if (task->done) {
      printf_log(LOG_DEBUG, "[%lu-dmiss%lu] '%s' has finished already!\n",
          *obs_it, *dmiss, task->name);
    }
    else {
      diff = task->count - ctx->last_counter;
      if (diff)
        printf_log(LOG_DEBUG, "[%lu-dmiss%lu] Counter for %s is %lu (+%lu)\n",
            *obs_it, *dmiss, task->name, task->count, diff);
      ctx->last_counter = task->count;
    }

    if (options.with_global_lock)
      run_assert(0 == sem_post(&options.global_lock));
  }
}

void observer_loop(struct taskset *ts) {
  unsigned long obs_iterations; /* number of iterations */
  unsigned long dmiss;          /* number of deadline misses */
  struct timespec at;           /* activation time */
  struct timespec dl;           /* deadline */
  /*struct timespec t;*/

  pthread_setname_np(pthread_self(), "obs");
  printf_log(LOG_INFO, "Hello\n");

  /*t.tv_nsec = 1;
  t.tv_sec = 0;*/
  obs_iterations = 0;
  dmiss = 0;
  set_period_ns(&at, &dl, OBSERVER_DEFAULT_PERIOD_ns, OBSERVER_DEFAULT_PERIOD_ns);

  while (true) {
    observer_body(ts, &obs_iterations, &dmiss);

    /*clock_nanosleep(CLOCK_MONOTONIC, 0x0, &t, NULL);*/
    wait_for_period_ns(&at, &dl, OBSERVER_DEFAULT_PERIOD_ns);
    if (deadline_miss(&dl)) {
      dmiss ++;
    }
  }
}

void *observer_function(void* param) {
  observer_loop((struct taskset*) param);
  return NULL;
}

/* handle errors that may happen in observer_start */
#define handle_error_en(en, fname) \
  do { \
    printf_log_perror(LOG_WARNING, en, \
        "Error while calling function %s: ", fname); \
    return; \
  } while (0)

#define handle_error_en_clean(en, fname) \
  do { pthread_attr_destroy(&tattr); handle_error_en(en, fname); } while (0)

static const char *get_sched_policy_string(int policy) {
  switch (policy) {
    case SCHED_OTHER:           return "SCHED_OTHER";
    case SCHED_FIFO:            return "SCHED_FIFO";
    case SCHED_RR:              return "SCHED_RR";
    default:                    return "UNKCNOWN SCHED POLICY";
  }
}

void observer_start(struct taskset *ts) {
  pthread_t tid;                        /* thread object */
  pthread_attr_t tattr;                 /* thread attributes */
  struct sched_param sched_param;       /* scheduling parameters */
  int policy;                   /* scheduling policy */
  int s;                        /* return value of called library functions */

  printf_log(LOG_DEBUG, "Starting creation of the observer\n");

  s = pthread_attr_init(&tattr);
  if (s) handle_error_en(s, "pthread_attr_init");

  s = pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
  if (s) handle_error_en_clean(s, "pthread_attr_setdetachstate");

  s = pthread_attr_setinheritsched(&tattr, PTHREAD_EXPLICIT_SCHED);
  if (s) handle_error_en_clean(s, "pthread_attr_setinheritsched");

  s = pthread_attr_setschedpolicy(&tattr, OBSERVER_SCHED_POLICY);

  s = pthread_attr_getschedpolicy(&tattr, &policy);
  if (s) handle_error_en_clean(s, "pthread_attr_getschedpolicy");
  printf_log(LOG_DEBUG, "Priorities shall be in the range [%d,%d] (using %s)\n",
      sched_get_priority_min(policy), sched_get_priority_max(policy),
      get_sched_policy_string(policy));

  sched_param.sched_priority = OBSERVER_PRIORITY;
  s = pthread_attr_setschedparam(&tattr, &sched_param);
  if (s) handle_error_en_clean(s, "pthread_attr_setschedparam");
  
  s = pthread_create(&tid, &tattr, observer_function, ts);
  if (s) handle_error_en_clean(s, "pthread_create");

  pthread_attr_destroy(&tattr);
}
#undef handle_error_en
