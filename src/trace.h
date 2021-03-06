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
 * This module implements a data structure for holding the schedule `trace`.
 *
 * A schedule trace is a simple array of `trace_node`s, each storing
 * info about a specific event.
 *
 * In order to prevent the GUI thread to slow down the observer, the trace
 * is not thread-safe: instead, it is expected that tasks coordinate insertions
 * while the GUI only reads.
 *
 * Note that the events[len] is the "current" event, if its `valid` flag is set.
 */

#ifndef __TRACE_H__
#define __TRACE_H__

#include <time.h>

#include "common.h"


#ifndef TRACE_SIZE
#define TRACE_SIZE 10000
#endif

enum {
  EVT_ACTIVATION,
  EVT_DEADLINE,
  EVT_START,
  EVT_COMPLETION,
  EVT_ACQUIRE,
  EVT_RELEASE,
  EVT_RUN
};

/** Converts an EVT_* constant to its corresponding string */
const char *evt_string(int evt);

struct trace_evt {
  bool valid;   /* Whether this event has been initialized */
  int type;     /* Type of event, among the EVT_* constants defined here. */
  int task;     /* Task index. -1 for idle task. */
  int res;      /* Resource used. 0 for no resource. */
  int count;    /* Number of consecutive equivalent events. */
  struct timespec time; /* Event timestamp, or start time for EVT_RUN */
  unsigned long tick;    /* Event tickstamp, or start tick for EVT_RUN */
};

struct trace {
  struct trace_evt events[TRACE_SIZE];
  int len;
};

/** Initialize the trace */
void trace_init(struct trace *tr);

/** Return the location for the next new node. If full, free up a slot first. */
struct trace_evt *trace_next(struct trace *tr);

/** Insert the node that was last returned by next_node */
void trace_next_add(struct trace *tr);



#endif
