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

#include "internals.h"


/**
 * Blocks until a character is read form keyboard, fills scan and ascii
 * with its scancode and ascii value
 */
static void get_keycodes(char *scan, char *ascii) {
  int k;
  k = readkey(); // block until a key is pressed
  *ascii = k; // get ascii code
  *scan = k >> 8; // get scan code
}


void get_user_input(struct guictx *ctx) {
  char scan, ascii;

  if (keyboard_needs_poll()) run_assert(poll_keyboard());

  if (keypressed()) {
    get_keycodes(&scan, &ascii);

    /* QUIT */
    if (scan == KEY_ESC || scan == KEY_Q) {
      printf_log(LOG_INFO, "Quitting gui...\n");
      ctx->exit = true;
    }
    /* ZOOM and PAN */
    else if (ascii == '+' || scan == KEY_UP || scan == KEY_P) {
      ctx->scale *= 2.0;
      printf_log(LOG_DEBUG, "Zoom in: scale now %lf px/ms\n", ctx->scale);
    }
    else if (ascii == '-' || scan == KEY_DOWN || scan == KEY_M) {
      ctx->scale /= 2.0;
      printf_log(LOG_DEBUG, "Zoom out: scale now %lf px/ms\n", ctx->scale);
    }
    else if (scan == KEY_LEFT) {
      ctx->disp_zero -= GUI_PAN / ctx->scale;
      printf_log(LOG_DEBUG, "Pan left: origin now at %d\n", ctx->disp_zero);
    }
    else if (scan == KEY_RIGHT) {
      ctx->disp_zero += GUI_PAN / ctx->scale;
      printf_log(LOG_DEBUG, "Pan right: origin now at %d\n", ctx->disp_zero);
    }
    /* TASKSET OPERATIONS */
    else if (scan == KEY_A) {
      if (! ctx->ts->activated) {
        printf_log(LOG_INFO, "Activating taskset...\n");
        taskset_activate(ctx->ts);
      }
      else {
        printf_log(LOG_INFO,"Taskset already running (or already finished).\n");
      }
    }
    else if (scan == KEY_S) {
      if (ctx->ts->activated  &&  ! ctx->ts->stopped) {
        printf_log(LOG_INFO, "Stopping taskeset...\n");
        taskset_quit(ctx->ts);
      }
      else {
        printf_log(LOG_INFO, "Can't stop taskset: not running.\n");
      }
    }
  }
}


#define HELP_LINE_LEN 30
#define HELP_LINE_SPACING 5

static const char help_lines[][HELP_LINE_LEN] = {
  "Useful keys:",
  " ESC Exit.",
  " ",
  " A   Activate taskset.",
  " S   Stop taskset.",
  " ",
  " ->  Scroll right",
  " <-  Scroll left",
  " +   Zoom in",
  " -   Zoom out",
  "",
};

void display_help(BITMAP *help_area) {
  const char *line;
  int ypos;
  int lineheight;

  lineheight = text_height(font) + HELP_LINE_SPACING;
  ypos = GUI_MARGIN;

  for (line = help_lines[0]; line[0] != '\0'; line += HELP_LINE_LEN) {
    textout_ex(help_area, font, line, GUI_MARGIN, ypos, HELP_TEXT_COL, -1);
    ypos += lineheight;
  }
}
