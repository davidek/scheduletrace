/*
 * Copyright 2015 Davide Kirchner
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <time.h>

#include "time_utils.h"
#include "periodic.h"


/* 
 * These functions are largely inspired by prof. Giorgio Buttazzo's slides
 * for the Real-Time Systems course.
 */


/**
 * Copy `ts` to `*td`
 */
void time_copy(struct timespec *td, const struct timespec *ts) {
  td->tv_sec = ts->tv_sec;
  td->tv_nsec = ts->tv_nsec;
}

/**
 * Add `ms` milliseconds to `*t`
 */
void time_add_ms(struct timespec *t, long ms) {
  t->tv_sec += ms / 1000;
  t->tv_nsec += (ms % 1000) * 1000000;
  if (t->tv_nsec > 1000000000) {
    t->tv_nsec -= 1000000000;
    t->tv_sec += 1;
  }
}

/**
 * Add `ns` nanoseconds to `*t`
 */
void time_add_ns(struct timespec *t, long ns) {
  t->tv_sec += ns / 1000000000;
  t->tv_nsec += (ns % 1000000000);
  if (t->tv_nsec > 1000000000) {
    t->tv_nsec -= 1000000000;
    t->tv_sec += 1;
  }
}


void set_period_ms(struct timespec *at, struct timespec *dl,
    long period, long deadline, const struct timespec *t0) {
  struct timespec t;

  if (t0 == NULL) {
    clock_gettime(CLOCK_MONOTONIC, &t);
    time_copy(at, &t);
    time_copy(dl, &t);
  }
  else {
    time_copy(at, t0);
    time_copy(dl, t0);
  }
  time_add_ms(at, period);
  time_add_ms(dl, deadline);
}

void set_period_ns(struct timespec *at, struct timespec *dl,
    long period, long deadline) {
  struct timespec t;

  clock_gettime(CLOCK_MONOTONIC, &t);
  time_copy(at, &t);
  time_copy(dl, &t);
  time_add_ns(at, period);
  time_add_ns(dl, deadline);
}


void wait_for_period_ms(struct timespec *at, struct timespec *dl, long period) {
  clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, at, NULL);
  time_add_ms(at, period);
  time_add_ms(dl, period);
}

void wait_for_period_ns(struct timespec *at, struct timespec *dl, long period) {
  clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, at, NULL);
  time_add_ns(at, period);
  time_add_ns(dl, period);
}


int deadline_miss(const struct timespec *dl) {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return (time_cmp(&now, dl) > 0);
}
