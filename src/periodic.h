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
 * Initialize absolute activation time and deadline
 */
void set_period(struct timespec *at, struct timespec *dl,
        int period, int deadline);

/**
 * Put the thread to sleep until next activation time, then shift both
 * activation time and deadline by one period.
 */
void wait_for_period(struct timespec *at, struct timespec *dl, int period);

/**
 * Whether the given deadline has been missed
 */
int deadline_miss(const struct timespec *dl);

#endif