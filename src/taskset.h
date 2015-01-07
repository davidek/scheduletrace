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

#ifndef __TASKSET_H__
#define __TASKSET_H__

#include "task.h"
#include "common.h"

#ifndef MAX_TASKSET_SIZE
#define MAX_TASKSET_SIZE 10
#endif

extern struct task_params *lonely_task;

struct taskset {
  struct task_params tasks[MAX_TASKSET_SIZE];
  int tasks_count;
};

int taskset_init_file(struct taskset* ts);

int taskset_start(struct taskset* ts);

#endif
