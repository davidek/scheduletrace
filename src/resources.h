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
 * This module deals with the [fake] resources managed by the tasks.
 *
 * Resource 0 is a special resource indicating "no resource",  but in
 * the API it is treated just like others.
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
  int prioceilings[MAX_RESOURCES - 1];
  pthread_mutex_t locks[MAX_RESOURCES - 1];
};


/** Initialize the struct for using with the `update` function */
void resources_init(struct resource_set *resources);

/**
 * Update the resources given that resource `r` will be used by a task
 * of priority `prio` 
 */
void resources_update(struct resource_set *resources, int r, int prio);

/** Initialize actual locks: after this, don't call `update` any more. */
void resources_locks_init(struct resource_set *resources);

void resources_locks_free(struct resource_set *resources);

void resource_acquire(struct resource_set *resources, int r);

void resource_release(struct resource_set *resources, int r);

#endif
