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

#include <time.h>

#include "periodic.h"


/* 
 * These functions are largely inspired by prof. Giorgio Buttazzo's slides
 * for the Real-Time Systems course.
 */


/**
 * Copy `ts` to `*td`
 */
void time_copy(struct timespec *td, struct timespec ts) {
  td->tv_sec = ts.tv_sec;
  td->tv_nsec = ts.tv_nsec;
}

/**
 * Add `ms` milliseconds to `*t`
 */
void time_add_ms(struct timespec *t, int ms) {
  t->tv_sec += ms / 1000;
  t->tv_nsec += (ms % 1000) * 1000000;
  if (t->tv_nsec > 1000000000) {
    t->tv_nsec -= 1000000000;
    t->tv_sec += 1;
  }
}

/**
 * Compare timespec: return 0 on equality, 1 if t1 > t2, -1 if t2 < t2
 */
int time_cmp(struct timespec t1, struct timespec t2) {
  if (t1.tv_sec > t2.tv_sec) return 1;
  if (t1.tv_sec < t2.tv_sec) return -1;
  if (t1.tv_nsec > t2.tv_nsec) return 1;
  if (t1.tv_nsec < t2.tv_nsec) return -1;
  return 0;
}


void set_period(struct timespec *at, struct timespec *dl,
    int period, int deadline) {
  struct timespec t;

  clock_gettime(CLOCK_MONOTONIC, &t);
  time_copy(at, t);
  time_copy(dl, t);
  time_add_ms(at, period);
  time_add_ms(dl, deadline);
}


void wait_for_period(struct timespec *at, struct timespec *dl, int period) {
  clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, at, NULL);
  time_add_ms(at, period);
  time_add_ms(dl, period);
}


int deadline_miss(const struct timespec *dl) {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return (time_cmp(now, *dl) > 0);
}
