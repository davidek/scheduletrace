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

#ifndef __GUI_INTERNALS_H__
#define __GUI_INTERNALS_H__

#include <pthread.h>
#include <allegro.h>

#include "../common.h"
#include "../taskset.h"

#include "../gui.h"


#define GUI_DEFAULT_ZOOM (200.0 / 1000)  /* px/ms */
#define GUI_PAN 50  /* px */
#define GUI_MAX_TRACELINE_HEIGHT 100

struct guictx {
  struct taskset *ts;   /* the observed taskset */
  pthread_t tid;        /* tid of the gui thread */
  volatile bool exit;   /* set to true to instruct main loop to exit */
  unsigned long dmiss;  /* counts deadline misses in the gui task */
  double scale;         /* current zoom level [px/ms] */
  long disp_zero;       /* time of the beginning of the time axis [ms start] */
  struct task_params *selected; /* currently selected task */
};


/**
 * Asynchronously checks if there is any pending user input and takes action.
 */
void get_user_input(struct guictx *ctx);  /* input.c */


/**
 * Display the input help message on the given bitmap.
 */
void display_help(BITMAP *help_area);  /* input.c */


/**
 * Display the execution trace in the given bitmap.
 */
void display_info(struct guictx *ctx, BITMAP *info_area);  /* info.c */


/** Return the color for the given resource */
int get_resource_color(int r);  /* trace.c */

/**
 * Display the execution trace in the given bitmap.
 */
void display_trace(struct guictx *ctx, BITMAP *main_area);  /* trace.c */


#endif
