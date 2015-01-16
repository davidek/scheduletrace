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
 * This module defines data structures for describing tasks and functions
 * to manage and run them.
 *
 * A task is described by a `struct task_params` instance:
 * It can be initialized either by manually setting its deadline, period,
 * priority and sections or by providing an initialization string.
 *
 * A task can then be created (i.e. its corresponding thread is spawned),
 * activated (i.e. it starts executing its body sections with the given period).
 *
 * Finally, a task can be quit gracefully by setting its `quit` flag.
 */

#ifndef __TASK_H__
#define __TASK_H__

#include <time.h>
#include <sched.h>
#include <pthread.h>
#include <semaphore.h>

#include "common.h"


#ifndef MAX_TASK_SECTIONS
#define MAX_TASK_SECTIONS 10
#endif

#define MAX_TASK_NAME_LEN 15  /* see PTHREAD_SETNAME_NP(3) */

#ifndef DEFAULT_TASK_PERIOD
#define DEFAULT_TASK_PERIOD 500
#endif

#ifndef DEFAULT_TASK_DEADLINE
#define DEFAULT_TASK_DEADLINE DEFAULT_TASK_PERIOD
#endif

#ifndef DEFAULT_TASK_PRIORITY
#define DEFAULT_TASK_PRIORITY 10
#endif

#ifndef TASK_SCHED_POLICY
#define TASK_SCHED_POLICY SCHED_RR
#endif

/**
 * Description of a section of a task consisting of a gaussian-distributed
 * number of simple operations to be done while locking a given resource.
 *
 * If `res` is zero, no resource is locked
 * The actual number of operations is max(0, random_gaussian(avg, dev))
 */
struct task_section {
  unsigned int res;     /* an id of the resource to be used, 0 for none */
  unsigned long avg;    /* average number of iterations */
  unsigned long dev;    /* standard deviation of number of iterations */
};

/**
 * Parameters required to start and run a periodic task
 */
struct task_params {
  /* To be set by the user before starting the thread */
  struct task_section sections[MAX_TASK_SECTIONS];  /* sequence of sections */
  int sections_count;           /* length of `sections` */
  unsigned int period;          /* in milliseconds */
  unsigned int deadline;        /* relative, in milliseconds */
  unsigned int priority;        /* in [0,99], allowed values depend on policy */

  /* Automatically set at creation/initialization time */
  char name[MAX_TASK_NAME_LEN + 1];     /* the thread name */  
  sem_t activation_sem; /* newly-created tasks will wait here for activation */
  bool activated;       /* becomes true after activation */
  pthread_t tid;        /* the thread id */

  /* Updated and used during execution */
  bool quit;            /* when true, instructs the task to stop gracefully */
  bool done;            /* becomes true after the task has stopped gracefully */
  int dmiss;            /* number of deadline misses */
  struct timespec at;   /* next activation time */
  struct timespec dl;   /* next absolute deadline */
  unsigned long count;  /* counts performed operations */
};

/**
 * Initializes a task_params with defaults.
 * Its behaviour can be tuned at compile time by setting DEFAULT_TASK_*
 * variables: PERIOD, DEADLINE and PRIORITY.
 */
void task_params_init(struct task_params *task);

/**
 * Initializes a task_params according to the given description string
 */
int task_params_init_str(struct task_params *task, const char *initstr);

/**
 * Create the thread for the task described by the given structure.
 * The scheduling policy to be used can be configured at compile time
 * by setting TASK_SCHED_POLICY, which defaults to SCHED_RR
 */
void task_create(struct task_params *task);

/**
 * Activate the task
 */
void task_activate(struct task_params *task);

/**
 * Write a string representation of the task into *str.
 * 
 * `verbosity` controls the level of details: 1, 2, 3
 */
void task_str(char *str, int len, const struct task_params *task, int verbosity);

#endif
