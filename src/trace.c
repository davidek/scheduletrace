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

#include <assert.h>

#include "common.h"
#include "trace.h"


const char *evt_string(int evt) {
  switch (evt) {
    case EVT_ACTIVATION:        return "ACTIVATION";
    case EVT_DEADLINE:          return "DEADLINE";
    case EVT_START:             return "START";
    case EVT_COMPLETION:        return "COMPLETION";
    case EVT_ACQUIRE:           return "ACQUIRE";
    case EVT_RELEASE:           return "RELEASE";
    case EVT_RUN:               return "RUN";
    default:                    return "ERROR-NO_SUCH_EVENT";
  }
}

static inline int next(int i) {
  return (i + 1) % TRACE_SIZE;
}

static void trace_evt_print(struct trace_evt *evt) {
  if (options.tracefile != NULL)
    fprintf(options.tracefile,
        "TRACE: [%lld.%.9ld][tick=%lu] %s task=%d R%d (x%u)\n",
        (long long) evt->time.tv_sec, evt->time.tv_nsec, evt->tick,
        evt_string(evt->type), evt->task, evt->res, evt->count);

  printf_log(LOG_DEBUG,
      "TRACE: [%lld.%.9ld][tick=%lu] %s task=%d R%d (x%u)\n",
      (long long) evt->time.tv_sec, evt->time.tv_nsec, evt->tick,
      evt_string(evt->type), evt->task, evt->res, evt->count);
}

void trace_init(struct trace *tr) {
  tr->first = 0;
  tr->last = 0;
}

struct trace_evt *trace_next(struct trace *tr) {
  if (next(tr->last) == tr->first) {
    /* tr->events[tr->first].valid = false; */
    tr->first = next(tr->first);
  }
  assert(next(tr->last) != tr->first);
  return &tr->events[next(tr->last)];
}

void trace_next_add(struct trace *tr) {
  assert(next(tr->last) != tr->first);
  tr->last = next(tr->last);
  trace_evt_print(&tr->events[tr->last]);
}
