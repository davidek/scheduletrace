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


/** Holds observer's parameters across activations. */
struct obs_ctx {
  unsigned long activations;    /* number of activations */
  unsigned int dmiss;           /* deadline misses */
  bool quit;
};

/* returns whether the task performed any operation */
static bool observe_running_task(struct task_params *task, struct obs_ctx *ctx){
  struct counter_set diff;

  assert(task->resources != NULL);
  /* diff = counters; diff -= observed; observed += diff;

     This way, "counters" is read only _once_,
     which should make this survive even if not semaphore-protected
  */
  counter_set_cp(&task->counters, &diff, task->resources->len);
  counter_set_decrement(&diff,  &task->observed, task->resources->len);
  counter_set_increment(&task->observed, &diff, task->resources->len);

  if (diff.tot) {
    printf_log(LOG_DEBUG, "[%lu-dmiss%lu] Counter for %s is %lu (+%lu)\n",
        ctx->activations,ctx->dmiss, task->name, task->counters.tot, diff.tot);
  }

  return (diff.tot > 0);
}

static void observer_body(struct taskset *ts, struct obs_ctx *ctx) {
  int i;
  struct task_params *task;
  int finished_tasks = 0;
  int executed_tasks = 0;

  ctx->activations ++;

  for (i = 0; i < ts->tasks_count; i++) {
    task = &ts->tasks[i];

    if (options.with_global_lock)
      run_assert(0 == sem_wait(&options.global_lock));

    if (! task->activated) {
      /* printf_log(LOG_DEBUG, "'%s' not active yet.\n", task->name); */
    }
    else if (task->done) {
      finished_tasks ++;
      printf_log(LOG_DEBUG, "[%lu-dmiss%lu] '%s' has finished already!\n",
          ctx->activations, ctx->dmiss, task->name);
    }
    else {
      if (observe_running_task(task, ctx))
        executed_tasks ++;
    }

    if (options.with_global_lock)
      run_assert(0 == sem_post(&options.global_lock));
  }

  if (executed_tasks > 1) {
    printf_log(LOG_DEBUG, "Doh! %d tasks ran while observer was asleep.\n",
        executed_tasks);
  }

  if (finished_tasks == ts->tasks_count) {
    ctx->quit = true;
    printf_log(LOG_INFO, "Quitting because all tasks are done.\n");
  }
}

static void observer_loop(struct taskset *ts) {
  struct timespec at;   /* activation time */
  struct timespec dl;   /* deadline */
  struct obs_ctx ctx;

  pthread_setname_np(pthread_self(), "obs");
  printf_log(LOG_INFO, "Observer started!\n");

  ctx.activations = 0;
  ctx.dmiss = 0;
  ctx.quit = false;

  set_period_ns(&at, &dl, OBSERVER_DEFAULT_PERIOD_ns, OBSERVER_DEFAULT_PERIOD_ns);

  do {
    observer_body(ts, &ctx);

    wait_for_period_ns(&at, &dl, OBSERVER_DEFAULT_PERIOD_ns);
    if (deadline_miss(&dl)) {
      ctx.dmiss ++;
    }
  } while (! ctx.quit);
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
    default:                    return "UNKNOWN SCHED POLICY";
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
