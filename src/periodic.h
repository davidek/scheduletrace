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

#ifndef __PERIODIC_H__
#define __PERIODIC_H__

/**
 * Initialize absolute activation time and deadline.
 * If not NULL, t0 is used as first activation in place of the current timestamp
 * If not zero, phase [ms] delays the first activation.
 */
void set_period_ms(struct timespec *at, struct timespec *dl,
        long period, long deadline, const struct timespec *t0, long phase);

void set_period_ns(struct timespec *at, struct timespec *dl,
        long period, long deadline);

/**
 * Put the thread to sleep until next activation time, then shift both
 * activation time and deadline by one period.
 */
void wait_for_period_ms(struct timespec *at, struct timespec *dl, long period);

void wait_for_period_ns(struct timespec *at, struct timespec *dl, long period);

/**
 * Whether the given deadline has been missed
 */
int deadline_miss(const struct timespec *dl);

#endif
