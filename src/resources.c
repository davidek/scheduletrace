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

#include <string.h>

#include "resources.h"
#include "common.h"


/** Initializes a lock according to compile- and run-time parameters */
void lock_init(pthread_mutex_t *lock) {

}

void resources_init(struct resource_set *resources, int len) {
  int r;

  resources->len = len;
  for (r = 1;  r < len; r++) {
    lock_init(&resources->locks[r-1]);
  }
}

void resources_free(struct resource_set *resources) {
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
