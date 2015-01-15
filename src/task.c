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

#include <errno.h>
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>

#include "task.h"
#include "periodic.h"
#include "common.h"


/* The task body, which shall be executed at every activation of the task */
void task_body(struct task_params* task) {
  int s;        /* section index */
  unsigned long i;      /* operation counter */

  printf_log(LOG_INFO, "Hey! period=%ld\n", task->period);
  for (s = 0; s < task->sections_count; s++) {
    for (i = 0; i < task->sections[s].avg; i++) {
      task->count ++;
      if (i % 100000 == 0) {
        printf_log(LOG_DEBUG, "%d\n", i/100000);
      }
    }
  }
}


/* Implememntation of the task */
void task_loop(struct task_params* task) {
  struct timespec at;
  struct timespec dl;
  int s;

  pthread_setname_np(pthread_self(), task->name);

  s = sem_wait(&task->activation_sem);
  if (s < 0) {
    printf_log_perror(LOG_WARNING, errno,
        "Task activation failed: sem_wait returned error: ");
    return;
  }
  task->activated = true;
  printf_log(LOG_INFO, "Activated!\n");

  set_period_ms(&at, &dl, task->period, task->deadline);

  while (! task->quit) {
    task_body(task);

    wait_for_period_ns(&at, &dl, task->period);
    if (deadline_miss(&dl)) {
      task->dmiss ++;
    }
  }

  task->done = true;
}


/* Wrapper around task_loop with a signature compatible with pthread_create */
void *task_function(void* task) {
  task_loop((struct task_params*) task);
  return NULL;
}


/* documented in header file */
void task_params_init(struct task_params* task) {
  static int task_count = 0;    /* enumerate created tasks */

  snprintf(task->name, MAX_TASK_NAME_LEN + 1, "task%d", task_count);
  task_count ++;

  task->sections_count = 0;
  task->period = DEFAULT_TASK_PERIOD;
  task->deadline = DEFAULT_TASK_DEADLINE;
  task->priority = DEFAULT_TASK_PRIORITY;

  task->activated = false;
  task->quit = false;
  task->done = false;
  task->dmiss = 0;
  task->count = 0;
}


int task_params_init_str(struct task_params *task, const char *initstr) {
  int n = -1;   /* stores the number of chars read */
  struct task_section *sect;    /* the section currently being parsed */

  task_params_init(task);

  sscanf(initstr, " T=%u,D=%u,pr=%u,[%n",
      &task->period, &task->deadline, &task->priority, &n);
  if (n < 0) {
    printf_log(LOG_WARNING,
        "Error while parsing (first part of) task string \"%s\"", initstr);
    return 1;
  }

  while (n >= 0  &&  task->sections_count < MAX_TASK_SECTIONS) {
    initstr += n;
    sect = &task->sections[task->sections_count];
    task->sections_count ++;
    n = -1;
    sscanf(initstr, "(R%u,avg=%lu,dev=%lu)%n",
        &sect->res, &sect->avg, &sect->dev, &n);
  }
  task->sections_count --;      /* was incremented once too much in the loop */

  n = -1;
  sscanf(initstr, "]%n", &n);
  if (n < 0) {
    printf_log(LOG_WARNING,
        "Error while parsing task sections starting from \"%s\""
        " (%d sections parsed successfully). Too many sections?\n",
        initstr, task->sections_count);
    task->sections_count = 0;
    return 1;
  }

  return 0;
}


void task_str(char *str,int len, const struct task_params *task, int verbosity){
  int n,        /* number of chars consumed by subsequent calls to sprintf */
      i;        /* loop index for iterating task sections */

  n = snprintf(str, len,
      "Task <%s>:\n  T=%u ms, D=%u ms, prio=%u;",
      task->name, task->period, task->deadline, task->priority);
  len -= n; str += n; assert(len > 0);

  if (verbosity >= 1) {
    n = snprintf(str, len,
        "\n  active=%d, quit=%d, done=%d, dmiss=%d, count=%lu, %u section[s];",
        task->activated, task->quit, task->done, task->dmiss, task->count,
        task->sections_count);
    len -= n; str += n; assert(len > 0);
  }

  if (verbosity >= 2) {
    for (i = 0; i < task->sections_count; i++) {
      n = snprintf(str, len,
          "\n  (R%u,avg=%lu,dev=%lu)",
          task->sections[i].res, task->sections[i].avg, task->sections[i].dev);
      len -= n; str += n; assert(len > 0);
    }
  }
}


/* handle errors that may happen in task_create */
#define handle_error_en(en, fname, tname) \
  do { \
    printf_log_perror(LOG_WARNING, en, \
        "Couldn't start `%s`: Got an error while calling function %s: ", \
        tname, fname); \
    return; \
  } while (0)

#define handle_error_en_clean(en, fname, tname) \
  do { pthread_attr_destroy(&tattr); handle_error_en(en, fname, tname); \
  } while (0)


static const char *get_sched_policy_string(int policy) {
  switch (policy) {
    case SCHED_OTHER:   return "SCHED_OTHER";
    case SCHED_FIFO:    return "SCHED_FIFO";
    case SCHED_RR:      return "SCHED_RR";
    default:            return "UNKNOWN SCHED POLICY";
  }
}


/* documented in header file */
void task_create(struct task_params *task) {
  pthread_attr_t tattr;                 /* thread attributes */
  struct sched_param sched_param;       /* scheduling parameters */
  int policy;                   /* scheduling policy */
  int s;                        /* return value of called library functions */

  printf_log(LOG_DEBUG, "Starting creation of %s\n", task->name);

  sem_init(&task->activation_sem, 0, 0);

  s = pthread_attr_init(&tattr);
  if (s) handle_error_en(s, "pthread_attr_init", task->name);

  s = pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
  if (s) handle_error_en_clean(s, "pthread_attr_setdetachstate", task->name);

  s = pthread_attr_setinheritsched(&tattr, PTHREAD_EXPLICIT_SCHED);
  if (s) handle_error_en_clean(s, "pthread_attr_setinheritsched", task->name);

  s = pthread_attr_setschedpolicy(&tattr, TASK_SCHED_POLICY);
  if (s) handle_error_en_clean(s, "pthread_attr_setschedpolicy", task->name);

  s = pthread_attr_getschedpolicy(&tattr, &policy);
  if (s) handle_error_en_clean(s, "pthread_attr_getschedpolicy", task->name);
  printf_log(LOG_DEBUG, "Priorities shall be in the range [%d,%d] (using %s)\n",
      sched_get_priority_min(policy), sched_get_priority_max(policy),
      get_sched_policy_string(policy));

  sched_param.sched_priority = task->priority;
  s = pthread_attr_setschedparam(&tattr, &sched_param);
  if (s) handle_error_en_clean(s, "pthread_attr_setschedparam", task->name);
  
  s = pthread_create(&task->tid, &tattr, task_function, task);
  if (s) handle_error_en_clean(s, "pthread_create", task->name);

  pthread_attr_destroy(&tattr);
}
#undef handle_error_en


void task_activate(struct task_params *task) {
  sem_post(&task->activation_sem);
}
