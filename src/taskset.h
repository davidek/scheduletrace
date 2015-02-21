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
 * This module provides a `struct taskset` and the functions to operate on it.
 *
 * The basic functionality is to collect a set of tasks and operate collectively
 * on all of its elements.
 *
 * A taskset also holds an instance of the observer context for each task.
 */

#ifndef __TASKSET_H__
#define __TASKSET_H__

#include <time.h>
#include <pthread.h>
#include <semaphore.h>

#include "common.h"
#include "resources.h"
#include "idle.h"
#include "task.h"
#include "trace.h"


#ifndef MAX_TASKSET_SIZE
#define MAX_TASKSET_SIZE 20
#endif

struct taskset {
  int tasks_count;
  struct task tasks[MAX_TASKSET_SIZE];
  struct idle_task idle;

  struct resource_set resources;

  sem_t task_lock;      /* mutex protecting writes to tick and trace */
  unsigned long tick;   /* global taskset tick */
  struct trace trace;   /* the event trace */
  struct trace_evt *next_evt;   /* the next event, to be added when ready */

  bool activated;       /* whether the taskset has been activated */
  bool stopped;         /* whether the taskset has been instructed to quit */

  struct timespec t0;   /* time when taskset_activate is called */
};

void taskset_init(struct taskset* ts);

int taskset_init_file(struct taskset* ts);

int taskset_create(struct taskset* ts);

void taskset_activate(struct taskset* ts);

void taskset_print(const struct taskset* ts);

void taskset_quit(struct taskset *ts);

void taskset_join(struct taskset *ts);

bool taskset_isactive(struct taskset *ts);

#endif
