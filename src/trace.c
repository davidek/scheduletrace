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

static void trace_evt_print(struct trace_evt *evt) {
  if (options.tracefile != NULL) {
    fprintf(options.tracefile,
        "TRACE: [%lld.%.9ld][tick=%lu] %s task=%d R%d (x%u)\n",
        (long long) evt->time.tv_sec, evt->time.tv_nsec, evt->tick,
        evt_string(evt->type), evt->task, evt->res, evt->count);

    if (options.tracefile_flush) fflush(options.tracefile);
  }

  printf_log(LOG_DEBUG,
      "TRACE: [%lld.%.9ld][tick=%lu] %s task=%d R%d (x%u)\n",
      (long long) evt->time.tv_sec, evt->time.tv_nsec, evt->tick,
      evt_string(evt->type), evt->task, evt->res, evt->count);
  if (options.tracefile_flush) fflush(options.logfile);
}

void trace_init(struct trace *tr) {
  tr->len = 0;
  tr->events[0].valid = false;
}

struct trace_evt *trace_next(struct trace *tr) {
  if (tr->len + 1 < TRACE_SIZE)
    tr->events[tr->len + 1].valid = false;
  return &tr->events[tr->len];
}

void trace_next_add(struct trace *tr) {
  trace_evt_print(&tr->events[tr->len]);

  if (tr->len + 1 >= TRACE_SIZE) {
    printf_log(LOG_WARNING,
        "Trace is full, will stop tracing. You may want to recompile with a "
        "higher TRACE_SIZE");
    tr->events[tr->len].valid = false;
  }
  else {
    tr->len ++;
  }
}
