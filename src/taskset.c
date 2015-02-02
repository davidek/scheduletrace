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
 * Implementation of the API in "taskset.h"
 */

#include <stdlib.h>
#include <stdio.h>

#include "common.h"
#include "task.h"
#include "taskset.h"


static void resources_setup_from_tasks(struct taskset *ts) {
  int t, s;  /* loop indices for tasks and sections */

  resources_init(&ts->resources);

  for (t = 0; t < ts->tasks_count; t++) {
    for (s = 0; s < ts->tasks[t].sections_count; s++) {
      resources_update(&ts->resources,
          ts->tasks[t].sections[s].res, ts->tasks[t].priority);
    }
  }

  printf_log(LOG_INFO, "Taskset required %d resource[s] (including dummy R0)\n",
      ts->resources.len);

  resources_locks_init(&ts->resources);
}

void taskset_init(struct taskset *ts) {
  ts->tasks_count = 0;
}

int taskset_init_file(struct taskset* ts) {
  int s;                /* stores return status of functions */
  char *line = NULL;    /* pointer to the line buffer */
  size_t len = 0;       /* size of alloccated line buffer */
  ssize_t read;         /* number of read characters */

  taskset_init(ts);

  while ((read = getline(&line, &len, options.infile)) != -1
      && ts->tasks_count < MAX_TASKSET_SIZE)
  {
    if (read == 0 || line[0] == '#' || line[0] == '\n')
      continue;

    s = task_params_init_str(&ts->tasks[ts->tasks_count], line);
    if (s) {
      printf_log(LOG_WARNING,
          "Task parsing was unsuccessful, skipping task definition.\n");
    }
    else {
      ts->tasks[ts->tasks_count].resources = &ts->resources;
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

  resources_setup_from_tasks(ts);

  return 0;
}

int taskset_create(struct taskset *ts) {
  int i;

  for (i = 0; i < ts->tasks_count; i++) {
    task_create(&ts->tasks[i]);
  }

  return 0;
}

void taskset_activate(struct taskset *ts) {
  int i;

  for (i = 0; i < ts->tasks_count; i++) {
    task_activate(&ts->tasks[i]);
  }
}

void taskset_print(const struct taskset *ts) {
  int i;
  char str[10000];

  printf_log(LOG_INFO, "Taskset made of %d tasks.\n", ts->tasks_count);
  for (i = 0; i < ts->tasks_count; i++) {
    task_str(str, 1000, &ts->tasks[i], 2);
    printf_log(LOG_INFO, "%s\n", str);
  }
}

void taskset_quit(struct taskset *ts) {
  int i;

  for (i = 0; i < ts->tasks_count; i++) {
    ts->tasks[i].quit = true;
  }
}
