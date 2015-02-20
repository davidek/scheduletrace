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


void set_period_ms(struct timespec *at, struct timespec *dl,
    long period, long deadline, const struct timespec *t0, long phase) {
  struct timespec t;

  if (t0 == NULL)
    clock_gettime(CLOCK_MONOTONIC, &t);
  else
    time_cpy(&t, t0);

  time_add_ms(&t, phase);

  time_cpy(at, &t);
  time_cpy(dl, &t);

  time_add_ms(dl, deadline - period);
}


void wait_for_period_ms(struct timespec *at, struct timespec *dl, long period) {
  clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, at, NULL);
  time_add_ms(at, period);
  time_add_ms(dl, period);
}


int deadline_miss(const struct timespec *dl) {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return (time_cmp(&now, dl) > 0);
}
