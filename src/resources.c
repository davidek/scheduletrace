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
 * Implementation of the API in "resources.h"
 */

#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#include "resources.h"
#include "common.h"


/** Initializes a lock according to compile- and run-time parameters */
void lock_init(pthread_mutex_t *lock, int prioceiling) {
  int s;
  pthread_mutexattr_t mattr;

  s = pthread_mutexattr_init(&mattr);
  if (s) {
    printf_log_perror(LOG_WARNING, s,
        "Error in lock_init while calling pthread_mutexattr_init: ");
    return;
  }

  /* type: NORMAL / RECURSIVE / ERRORCHECK / DEFAULT */
  s = pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_ERRORCHECK);
  if (s) {
    printf_log_perror(LOG_WARNING, s,
        "Error in lock_init while calling pthread_mutexattr_settype: ");
    return;
  }

  /* TODO: options.mutex_protocol */
  s = pthread_mutexattr_setprotocol(&mattr, PTHREAD_PRIO_NONE);
  if (s) {
    printf_log_perror(LOG_WARNING, s,
        "Error in lock_init while calling pthread_mutexattr_setprotocol: ");
    return;
  }

  /*  TODO
  if (options.mutex_protocol == PTHREAD_PRIO_PROTECT) {
    s = pthread_mutexattr_setprioceiling(&mattr, prioceiling);
    if (s) {
      printf_log_perror(LOG_WARNING, s,
          "Error in lock_init while calling pthread_mutexattr_setprioceiling:");
      return;
    }
  }
  */

  s = pthread_mutex_init(lock, &mattr);
  if (s) {
    printf_log_perror(LOG_WARNING, s,
        "Error in lock_init while calling pthread_mutex_init: ");
    return;
  }

  s = pthread_mutexattr_destroy(&mattr);
  assert(s == 0);
}

void resources_init(struct resource_set *resources) {
  int r;

  resources->len = 0;
  for (r = 1; r < MAX_RESOURCES; r++) {
    resources->prioceilings[r-1] = -1;
  }
}

static inline int max(int a, int b) {
  return (a > b) ? a : b;
}

void resources_update(struct resource_set *resources, int r, int prio) {
  if (r >= MAX_RESOURCES) {
    printf_log(LOG_ERROR, "Resources number %d exceeds max resources (%d).\n",
        r, MAX_RESOURCES);
    exit(1);
  }

  resources->len = max(r + 1, resources->len);
  if (r > 0) {
    resources->prioceilings[r-1] = max(prio, resources->prioceilings[r-1]);
  }
}

void resources_locks_init(struct resource_set *resources) {
  int r;

  printf_log(LOG_INFO, "Initializing locks for resources.\n");
  for (r = 1;  r < resources->len; r++) {
    printf_log(LOG_DEBUG, "  Resource R%d with priority ceiling %d;\n",
        r, resources->prioceilings[r-1]);
    lock_init(&resources->locks[r-1], resources->prioceilings[r-1]);
  }
}

void resources_locks_free(struct resource_set *resources) {
  int r;
  int s;

  for (r = 1;  r < resources->len; r++) {
    s = pthread_mutex_destroy(&resources->locks[r-1]);
    if (s)
      printf_log_perror(LOG_WARNING, s, "Error in pthread_mutex_destroy: ");
  }
}

void resource_acquire(struct resource_set *resources, int r) {
  int s;

  if (r > 0) {
    s = pthread_mutex_lock(&resources->locks[r-1]);
    if (s)
      printf_log_perror(LOG_WARNING, s, "Error in pthread_mutex_lock: ");
  }
}

void resource_release(struct resource_set *resources, int r) {
  int s;

  if (r > 0) {
    s = pthread_mutex_unlock(&resources->locks[r-1]);
    if (s)
      printf_log_perror(LOG_WARNING, s, "Error in pthread_mutex_unlock: ");
  }
}

void counter_set_init(struct counter_set *counters) {
  memset(counters, 0, sizeof(struct counter_set));
}

void counter_set_cp(
    const struct counter_set *src, struct counter_set *dst, int len) {
  int r;

  dst->sections = src->sections;
  dst->tot = src->tot;
  for (r = 0; r < len; r++) {
    dst->acquirements[r] = src->acquirements[r];
    dst->operations[r] = src->operations[r];
    dst->releases[r] = src->releases[r];
  }
}

void counter_set_increment(
    struct counter_set *a, const struct counter_set *b, int len) {
  int r;

  a->sections += b->sections;
  a->tot += b->tot;
  for (r = 0; r < len; r++) {
    a->acquirements[r] += b->acquirements[r];
    a->operations[r] += b->operations[r];
    a->releases[r] += b->releases[r];
  }
}

void counter_set_decrement(
    struct counter_set *a, const struct counter_set *b, int len) {
  int r;

  a->sections -= b->sections;
  a->tot -= b->tot;
  for (r = 0; r < len; r++) {
    a->acquirements[r] -= b->acquirements[r];
    a->operations[r] -= b->operations[r];
    a->releases[r] -= b->releases[r];
  }
}
