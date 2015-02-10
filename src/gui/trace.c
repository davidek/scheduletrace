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

#include <limits.h>

#include "internals.h"


#define TIMELINE_H 40
#define TIMELINE_Y 15

#define LINESTART_X 40
#define LINEEND_X_ROFF 20
#define TICK_DISTANCE 50
#define TICK_LEN 5


#define TASKLINE_Y 50


/****** TIMELINE  ******/

/* "-1"-terminated array of possible ticks distance [ms] */
static const int POSSIBLE_TICKS[] = {
  1, 2, 5, 10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000, -1
};

/**
 * Given the scale, decides at which distance to print ticks.
 *
 * scale: the input scale in px/ms
 * [RET] time_dist: the distance between two ticks in ms
 */
static void scale2ticks(double scale, int *time_dist) {
  int i;        /* loop index */
  int base;     /* the non-rounded tick distance */
  int diff;     /* distance base - current_trial */
  int best;     /* smallest diff */
  int best_i;   /* index of smallest diff */

  assert(time_dist != NULL);

  base = TICK_DISTANCE / scale;

  best_i = -1;
  best = INT_MAX;
  for (i = 0; POSSIBLE_TICKS[i] != -1; i++) {
    diff = abs(base - POSSIBLE_TICKS[i]);
    if (diff < best) {
      best = diff;
      best_i = i;
    }
  }
  assert(best_i >= 0);

  *time_dist = POSSIBLE_TICKS[best_i];
  /* *px_dist = (*time_dist) * scale; */
}

static int ceildiv(int a, int b) {
  /* return (a / b) + !!(a % b);  does not work with negatives */
  int q;

  q = a / b;
  return q + !!(q * b < a);
}
/** return the smallest multiple of `a` grater or equal to `b` */
static int nextmult(int a, int b) {
  return ceildiv(b, a) * a; 
}

static void disp_timeline(struct guictx *ctx, BITMAP *area, int y, bool nums) {
  int x, t;
  int time_end;
  int time_dist;
  char *unit;
  int unit_factor;

  scale2ticks(ctx->scale, &time_dist);

  if (time_dist % 1000) {
    unit = "ms";
    unit_factor = 1;
  }
  else {
    unit = "s";
    unit_factor = 1000;
  }

  if (nums) {
    textprintf_ex(area, font,
        1+text_length(font, "["), y - text_height(font) / 2, TEXT_COL, -1, "t");
    textprintf_ex(area, font,
        1, y + GUI_MARGIN + text_height(font) / 2, TEXT_COL, -1,
        "[%s]", unit);
  }

  hline(area, LINESTART_X, y + 0, area->w - LINEEND_X_ROFF, TEXT_COL);
  hline(area, LINESTART_X, y + 1, area->w - LINEEND_X_ROFF, TEXT_COL);

  time_end =
    ctx->disp_zero + (area->w - LINEEND_X_ROFF - LINESTART_X) / ctx->scale;

  for (t = nextmult(time_dist, ctx->disp_zero); t < time_end; t += time_dist) {
    x = LINESTART_X + (t - ctx->disp_zero) * ctx->scale;
    vline(area, x, y, y + TICK_LEN, TEXT_COL);
    if (nums)
      textprintf_centre_ex(area, font,
          x + 1, y + TICK_LEN + text_height(font) / 2, TEXT_COL, -1,
          "%d", t / unit_factor);
  }
}


/****** TRACE  ******/


void display_trace(struct guictx *ctx, BITMAP *area) {
  BITMAP *timeline_area, *trace_area;

  timeline_area = create_sub_bitmap(area,
      0, area->h - TIMELINE_H, area->w, TIMELINE_H);
  clear_to_color(timeline_area, BG_COL);

  trace_area = create_sub_bitmap(area,
      0, 0, area->w, area->h - TIMELINE_H);
  //clear_to_color(trace_area, COL_BLUE);

  disp_timeline(ctx, timeline_area, TIMELINE_Y, true);

  destroy_bitmap(timeline_area);
  destroy_bitmap(trace_area);
}
