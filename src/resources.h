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
 * This module deals with the [fake] resources managed by the observed tasks,
 * and the counters that represent the execution while operating on each.
 *
 * Resource 0 is a special resource indicating "no resource",  but in
 * the API and counters it is treated just like others.
 *
 * Function names should be self-explaining, most of them are couple of lines.
 *
 * NOTE: although provided here, `struct resource_set` should be treated as
 * an opaque type. This has also the advantage of avoiding index confusion,
 * as internally the `locks` array does not include resource 0.
 */

#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#include <pthread.h>

#ifndef MAX_RESOURCES
#define MAX_RESOURCES 10
#endif

struct resource_set {
  int len;
  pthread_mutex_t locks[MAX_RESOURCES - 1];
};

struct counter_set {
  // int len;      /* Number of resources */
                                                /* Counters incremented on: */
  unsigned long sections;                       /* - enter new section */
  unsigned long acquirements[MAX_RESOURCES];    /* - acquire resource r */
  unsigned long operations[MAX_RESOURCES];      /* - operate on resource r */
  unsigned long releases[MAX_RESOURCES];        /* - release resource r */
  unsigned long tot;                            /* - any of the previous */
};

void resources_init(struct resource_set *resources, int len);

void resources_free(struct resource_set *resources);

void resource_acquire(struct resource_set *resources, int r);

void resource_release(struct resource_set *resources, int r);

void counter_set_init(struct counter_set *counters);

void counter_set_cp(
    const struct counter_set *src, struct counter_set *dst, int len);

/** performs `a += b`  element-by-element */
void counter_set_increment(
    struct counter_set *a, const struct counter_set *b, int len);

/** performs `a -= b`  element-by-element */
void counter_set_decrement(
    struct counter_set *a, const struct counter_set *b, int len);

#endif
