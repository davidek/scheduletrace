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
 * This file holds the main GUI entry point and the code to start and run
 * the GUI thread.
 */

#include <pthread.h>
#include <sched.h>
#include <allegro.h>

#include "../common.h"
#include "../periodic.h"
#include "internals.h"


/* Split the screen into sub-bitmaps */
static void screen_split(
    BITMAP **main_area, BITMAP **info_area, BITMAP **help_area)
{
  int vsplit, hsplit;

  vsplit = SCREEN_W * 0.7;
  assert(vsplit != 0);  /* Should be a user-friendlier error... */

  vline(screen, vsplit, 0, SCREEN_H, SPLITLINE_COL);

  *main_area = create_sub_bitmap(screen, 0, 0, vsplit, SCREEN_H);

  hsplit = SCREEN_H * .5;

  hline(screen, vsplit, hsplit, SCREEN_W, SPLITLINE_COL);

  *info_area = create_sub_bitmap(screen,
      vsplit + 1, 0, SCREEN_W - vsplit - 1, hsplit);
  *help_area = create_sub_bitmap(screen,
      vsplit + 1, hsplit + 1, SCREEN_W - vsplit - 1, SCREEN_H);
}


static void screen_split_cleanup(
    BITMAP *main_area, BITMAP *info_area, BITMAP *help_area)
{
  if (main_area != NULL) destroy_bitmap(main_area);
  if (info_area != NULL) destroy_bitmap(info_area);
  if (help_area != NULL) destroy_bitmap(help_area);
}


static void gui_thread_main(struct guictx *ctx) {
  int s;
  BITMAP *main_area, *info_area, *help_area;
  struct timespec at;
  struct timespec dl;

  s = pthread_setname_np(pthread_self(), "gui");
  if (s) {
    printf_log_perror(LOG_ERROR, s,
        "GUI thread activation failed: pthread_setname_np returned error: ");
    return;
  }

  screen_split(&main_area, &info_area, &help_area);
  if (main_area == NULL || info_area == NULL || help_area == NULL) {
    printf_log(LOG_ERROR, "An error occurred in screen_split.\n");
    return;
  }

  //clear_to_color(main_area, COL_RED);
  //clear_to_color(info_area, COL_GREEN);
  //clear_to_color(help_area, COL_BLUE);

  display_help(help_area);

  //sleep(2);
  //taskset_quit(ctx->ts);
  //taskset_activate(ctx->ts);

  set_period_ms(&at, &dl, GUI_PERIOD, GUI_DEADLINE, NULL, 0);

  while (! ctx->exit) {
    wait_for_period_ms(&at, &dl, GUI_PERIOD);
    if (deadline_miss(&dl)) {
      ctx->dmiss ++;
      printf_log(LOG_INFO, "GUI Deadline miss! (so far: %d)\n", ctx->dmiss);
    }

    get_user_input(ctx);
    display_info(ctx, info_area);
    display_trace(ctx, main_area);
    //s = gui_update();
    //if (s != 0) break;
  }

  if (! ctx->ts->activated) {
    /* Tasks are still waiting for activation: unlock them so they exit */
    printf_log(LOG_INFO, "Destroying the taskset, which didn'e even start.\n");
    taskset_quit(ctx->ts);
    taskset_activate(ctx->ts);
  }
  else if (! ctx->ts->stopped) {
    printf_log(LOG_INFO, "Stopping the taskset before quitting...\n");
    taskset_quit(ctx->ts);
  }
  printf_log(LOG_INFO, "Waiting for the taskset to finish...\n");
  taskset_join(ctx->ts);

  screen_split_cleanup(main_area, info_area, help_area);
}


static void *gui_thread_function(void *params) {
  gui_thread_main((struct guictx *)params);
  return NULL;
}


/* handle errors that may happen in task_create */
#define handle_error(en, fname) \
  do { \
    printf_log_perror(LOG_WARNING, en, \
        "Couldn't start GUI thread: Got an error while calling function %s: ", \
        fname); \
    return; \
  } while (0)

#define handle_error_clean(en, fname) \
  do { pthread_attr_destroy(&tattr); handle_error(en, fname); } while (0)

/** Create the thread for GUI management */
static void gui_thread_create(struct guictx *ctx) {
  pthread_attr_t tattr;                 /* thread attributes */
  struct sched_param sched_param;       /* scheduling parameters */
  int s;                        /* return value of called library functions */

  s = pthread_attr_init(&tattr);
  if (s) handle_error(s, "pthread_attr_init");

  s = pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_JOINABLE);
  if (s) handle_error_clean(s, "pthread_attr_setdetachstate");

  s = pthread_attr_setinheritsched(&tattr, PTHREAD_EXPLICIT_SCHED);
  if (s) handle_error_clean(s, "pthread_attr_setinheritsched");

  s = pthread_attr_setschedpolicy(&tattr, SCHED_RR);
  if (s) handle_error_clean(s, "pthread_attr_setschedpolicy");

  sched_param.sched_priority = GUI_THREAD_PRIORITY;
  s = pthread_attr_setschedparam(&tattr, &sched_param);
  if (s) handle_error_clean(s, "pthread_attr_setschedparam");
  
  s = pthread_create(&ctx->tid, &tattr, gui_thread_function, (void *) ctx);
  if (s) handle_error_clean(s, "pthread_create");

  pthread_attr_destroy(&tattr);
}
#undef handle_error
#undef handle_error_clean


/* Required by the close_button_handler */
static struct guictx *global_ctx;


void close_button_handler(void) {
  global_ctx->exit = true;
}
END_OF_FUNCTION(close_button_handler)


/** Fully initialize the graphics library and start the GUI threads */
static void graphics_init() {
  int s;

  s = allegro_init();
  if (s) {
    printf_log(LOG_ERROR, "Allegro library refused to start.\n");
    exit(1);
  }

  set_color_depth(16);  /* 8,15,16,24,32 */

  set_window_title("Scheduletrace - Schedule visualizer for Linux");

  LOCK_FUNCTION(close_button_handler);
  set_close_button_callback(close_button_handler);

  s = set_gfx_mode(GFX_AUTODETECT_WINDOWED, options.gui_w, options.gui_h, 0, 0);
  if (s) {
    printf_log(LOG_ERROR, "Allegro library failed in set_gfx_mode.\n");
    exit(1);
  }

  s = install_keyboard();
  if (s) {
    printf_log(LOG_ERROR, "Allegro library failed in install_keyboard.\n");
    exit(1);
  }
}


/* Main entry-point */
void gui_run(struct taskset *ts) {
  int s;
  struct guictx ctx;

  ctx.exit = false;
  ctx.ts = ts;
  ctx.dmiss = 0;
  ctx.scale = GUI_DEFAULT_ZOOM;
  ctx.disp_zero = 0;
  ctx.selected = &ts->tasks[0];

  global_ctx = &ctx;

  graphics_init();

  gui_thread_create(&ctx);

  s = pthread_join(ctx.tid, NULL);
  if (s) {
    printf_log_perror(LOG_ERROR, s,
        "Error calling pthread_join on the gui thread: ");
  }

  allegro_exit();
}

