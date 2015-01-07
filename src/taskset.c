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

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>

#include "task.h"
#include "taskset.h"


struct task_params *lonely_task;

int taskset_init_file(struct taskset* ts) {
  int s;                /* stores return status of functions */
  char *line = NULL;    /* pointer to the line buffer */
  size_t len = 0;       /* size of alloccated line buffer */
  ssize_t read;         /* number of read characters */

  while ((read = getline(&line, &len, options.infile)) != -1
      && ts->tasks_count < MAX_TASKSET_SIZE)
  {
    if (read == 0 || line[0] == '#')
      continue;

    s = task_params_init_str(&ts->tasks[ts->tasks_count], line);
    if (s) {
      printf_log(LOG_WARNING,
          "Task parsing was unsuccessful, skipping task definition.\n");
    }
    else {
      ts->tasks_count ++;
    }
  }

  if (line) free(line);
  fclose(options.infile);

  if (read != -1) {
    printf_log(LOG_WARNING,
        "Maximum number of tasks reached, some may have been skipped!\n",
        MAX_TASKSET_SIZE);
  }

  return 0;
}

int taskset_start(struct taskset* ts) {
  int i;

  for (i = 0; i < ts->tasks_count; i++) {
    task_start(&ts->tasks[i]);
  }

  return 0;
}

