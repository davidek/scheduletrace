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
 * This module provides a simplified task interface for the observer thread.
 *
 * Most of the observer's parameters are defined at compile time, making
 * this API quite minimal.
 *
 * This module also defines the observer context.
 */

#ifndef __OBSERVER_H__
#define __OBSERVER_H__

#include <sched.h>

#define OBSERVER_SCHED_POLICY SCHED_RR

#define OBSERVER_PRIORITY 90

#ifndef OBSERVER_DEFAULT_PERIOD_ns
#define OBSERVER_DEFAULT_PERIOD_ns (10 * 1000)
#endif

/**
 * A data structure for storing a per-task persistent observer context
 *
 * No other thread but the observer shall ever edit one of these.
 */
struct observer_ctx {
  unsigned long last_counter;
};

#include "taskset.h"  /* deferred to prevent circular import problems */

void observer_ctx_init(struct observer_ctx *ctx);

void observer_start(struct taskset *ts);

#endif
