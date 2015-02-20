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
 * This module provides utility functions to operate on `timespec` instances
 */

#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <time.h>

/**
 * Compare timespec: return 0 on equality, 1 if t1 > t2, -1 if t2 < t2
 */
int time_cmp(const struct timespec *t1, const struct timespec *t2);

/**  */
long time_to_ms(const struct timespec *t);

/** Return time_to_ms(t1 - t2) */
long time_diff_ms(const struct timespec *t1, const struct timespec *t2);

#endif
