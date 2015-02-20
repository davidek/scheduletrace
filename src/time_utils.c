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
 * This file simply implements the API in time_utils.h
 */

#include <assert.h>

#include "time_utils.h"


/* 
 * Some of these functions are inspired by prof. Giorgio Buttazzo's slides
 * for the Real-Time Systems course.
 */


void time_cpy(struct timespec *td, const struct timespec *ts) {
  td->tv_sec = ts->tv_sec;
  td->tv_nsec = ts->tv_nsec;
}

int time_cmp(const struct timespec *t1, const struct timespec *t2) {
  if (t1->tv_sec > t2->tv_sec) return 1;
  if (t1->tv_sec < t2->tv_sec) return -1;
  if (t1->tv_nsec > t2->tv_nsec) return 1;
  if (t1->tv_nsec < t2->tv_nsec) return -1;
  return 0;
}


void time_add_ms(struct timespec *t, long ms) {
  t->tv_sec += ms / 1000;
  t->tv_nsec += (ms % 1000) * 1000000;
  if (t->tv_nsec > 1000000000) {
    t->tv_nsec -= 1000000000;
    t->tv_sec += 1;
  }
}


static long time_to_ms(const struct timespec *t) {
  return t->tv_sec * 1000 + t->tv_nsec / 1000000;
}


long time_diff_ms(const struct timespec *t1, const struct timespec *t2) {
  struct timespec diff;

  /* assert(time_cmp(t1, t2) >= 0); */

  diff.tv_sec = t1->tv_sec - t2->tv_sec;

  if (t1->tv_nsec < t2->tv_nsec) {
    diff.tv_nsec = 1000000000 + t1->tv_nsec - t2->tv_nsec;
    diff.tv_sec --;
  }
  else {
    diff.tv_nsec = t1->tv_nsec - t2->tv_nsec;
  }

  return time_to_ms(&diff);
}
