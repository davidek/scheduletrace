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
#include <math.h>

#include "../time_utils.h"
#include "../bsearch_left.h"
#include "internals.h"


#define LINESTART_X     40
#define LINEEND_X_ROFF  20

#define TIMELINE_H      40
#define TIMELINE_Y      15
#define TICK_DISTANCE   50
#define TICK_LEN        5
#define TICKS_PER_LABEL 2  /* One tick every TICKS_PER_LABEL will be labelled */

#define LOAD_H          80
#define LOAD_PLOT_H     60

#define TRACE_H         18
#define DEADLINE_H      (TRACE_H + 4)
#define ACTIVATION_H    (DEADLINE_H + 4)
#define ACT_DEADL_W     2


static const PALETTE PLOT_PALETTE = {
  {0xff, 0xff, 0x33},   /* R0 - no resource */
  {0xff, 0x00, 0x66},
  {0xff, 0x00, 0x00},
  {0x00, 0xff, 0xcb},
  {0xff, 0x66, 0x00},
  {0xcb, 0xff, 0x00},
  {0x00, 0xff, 0x66},
  {0x00, 0xcb, 0xff},
  {0x00, 0x66, 0xff},
  {0x00, 0x00, 0xff},
  {0x65, 0x00, 0xff},
  {0xcc, 0x00, 0xff},
  {0xff, 0x00, 0xcb},
  {0xff, 0xcc, 0x00},
  {0x65, 0xff, 0x00},
  {0xee, 0xee, 0xee},
  {0xee, 0xee, 0xee},
  {0xee, 0xee, 0xee},
  {0xee, 0xee, 0xee},
  {0xee, 0xee, 0xee},
  {0xee, 0xee, 0xee},
};

/* Return the right limit of the interesting time */
static long time_limit(struct guictx *ctx) {
  const struct trace_evt *evt;

  if (! ctx->ts->stopped) {
    return LONG_MAX;
  }
  else {
    evt = &ctx->ts->trace.events[ctx->ts->trace.len];
    if (! evt->valid)
      evt = &ctx->ts->trace.events[ctx->ts->trace.len - 1];

    assert(evt->valid);
    return time_diff_ms(&evt->time, &ctx->ts->t0);
  }
}

static long time_to_px(struct guictx *ctx, int width, long time) {
  return (time - ctx->disp_zero) * ctx->scale;
}

static long px_to_time(struct guictx *ctx, int width, long px) {
  return px / ctx->scale + ctx->disp_zero;
}

static long max_disp_time(struct guictx *ctx, int net_width) {
  return ctx->disp_zero + net_width / ctx->scale;
}

static int evt_time_cmp(const void *time_key, const void *evt_item) {
  return time_cmp(time_key, &((struct trace_evt *)evt_item)->time);
}

/* Return the index of the latest event preceding the given time  */
int evt_preceding(struct guictx *ctx, long time_ms) {
  struct timespec t;

  time_cpy(&t, &ctx->ts->t0);
  time_add_ms(&t, time_ms);

  return bsearch_left(&t, &ctx->ts->trace.events, ctx->ts->trace.len,
      sizeof(struct trace_evt), evt_time_cmp);
}

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
static void scale2ticks(double scale, long *time_dist) {
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
static int nextmult(int a, int b, int phase) {
  return ceildiv(b - phase, a) * a + phase; 
}

static void disp_timeline(struct guictx *ctx, BITMAP *area, int y,
    bool show_scale, bool with_offset) {
  int x, t;
  long time_end;
  long time_dist;
  const char *unit;
  int unit_factor;
  int linestart_x;
  int lineend_x_roff;

  /* adjust the fact that the lower timeline area is wider than the tracelines*/
  linestart_x = (with_offset ? LINESTART_X : 0);
  lineend_x_roff = (with_offset ? LINEEND_X_ROFF : 0);

  scale2ticks(ctx->scale, &time_dist);

  if (time_dist % 1000) {
    unit = "ms";
    unit_factor = 1;
  }
  else {
    unit = "s";
    unit_factor = 1000;
  }

  if (show_scale) {
    textprintf_ex(area, font,
        1+text_length(font, "["), y - text_height(font) / 2, TEXT_COL, -1, "t");
    textprintf_ex(area, font,
        1, y + GUI_MARGIN + text_height(font) / 2, TEXT_COL, -1,
        "[%s]", unit);
  }

  hline(area, linestart_x, y + 0, area->w - lineend_x_roff, TEXT_COL);
  hline(area, linestart_x, y + 1, area->w - lineend_x_roff, TEXT_COL);

  time_end = max_disp_time(ctx, area->w - linestart_x - lineend_x_roff);
  for (t = nextmult(time_dist, ctx->disp_zero, 0); t < time_end; t += time_dist)
  {
    x = linestart_x + (t - ctx->disp_zero) * ctx->scale;

    vline(area, x, y, y + TICK_LEN, TEXT_COL);
    if (show_scale && !(t / time_dist % TICKS_PER_LABEL))
      textprintf_centre_ex(area, font,
          x + 1, y + TICK_LEN + text_height(font) / 2, TEXT_COL, -1,
          "%d", t / unit_factor);
  }
}


/****** TRACE  ******/

/* Return the height of each trace line */
static int get_line_height(struct guictx *ctx, int tot_height) {
  int ret;

  ret = tot_height / (ctx->ts->tasks_count + 1);
  if (ret > GUI_MAX_TRACELINE_HEIGHT) ret = GUI_MAX_TRACELINE_HEIGHT;
  return ret;
}

/** Return the color for the given resource */
int get_resource_color(int r) {
  RGB ret;

  select_palette(PLOT_PALETTE);
  get_color(r, &ret);
  unselect_palette();
  return makecol(ret.r, ret.g, ret.b);
}

/** Draw a single event */
static void disp_evt(struct guictx *ctx, BITMAP *area,
    const struct trace_evt *evt, long start_time,long end_time, int line_height)
{
  int startpx, endpx;

  startpx = time_to_px(ctx, area->w, start_time);
  endpx = time_to_px(ctx, area->w, end_time);

  /* void rectfill(BITMAP *bmp, int x1, int y1, int x2, int y2, int color); */
  rectfill(area,
      startpx,
      (evt->task+1 + 1) * line_height - GUI_MARGIN - 1 + (
        evt->type == EVT_RUN ? 0 : 1),
      endpx,
      (evt->task+1 + 1) * line_height - GUI_MARGIN - 1 - TRACE_H,
      get_resource_color(evt->res));
}

/* Draw the activations and deadlines for the given task */
static void disp_at_dt(
    struct guictx *ctx, BITMAP *area, struct task *task, int line_height)
{
  long time;
  long time_upper_limit = time_limit(ctx);
  int px;

  /* Activation times */
  for (time = nextmult(task->period, ctx->disp_zero, task->phase);
      time <= max_disp_time(ctx, area->w);  time += task->period)
  {
    if (time >= 0 && time < time_upper_limit) {
      px = time_to_px(ctx, area->w, time);
      rectfill(area,
          px, (task->id+1 + 1) * line_height - GUI_MARGIN - 1,
          px + ACT_DEADL_W - 1,
          (task->id+1 + 1) * line_height - GUI_MARGIN - 1 - ACTIVATION_H,
          ACTIVATION_COL);
    }
  }
  /* Deadlines */
  for (time = nextmult(task->period, ctx->disp_zero,
                       task->phase + task->deadline - task->period);
      time <= max_disp_time(ctx, area->w);  time += task->period)
  {
    if (time >= 0  &&  time < time_upper_limit) {
      px = time_to_px(ctx, area->w, time);
      rectfill(area,
          px, (task->id+1 + 1) * line_height - GUI_MARGIN - 1,
          px + ACT_DEADL_W - 1,
          (task->id+1 + 1) * line_height - GUI_MARGIN - 1 - DEADLINE_H,
          DEADLINE_COL);
    }
  }
}

/* Display the task names */
static void disp_headings(struct guictx *ctx, BITMAP *area) {
  int t;

  if (! ctx->redraw) {
    return;
  }

  printf_log(LOG_DEBUG, "Re-drawing line headings...\n");
  clear_to_color(area, BG_COL);

  for (t = -1; t < ctx->ts->tasks_count; t++) {
    textprintf_ex(area, font,
        GUI_MARGIN, (t+1 + 1) * get_line_height(ctx, area->h) - GUI_MARGIN -1 - text_height(font),
        TEXT_COL, BG_COL,
        (t >= 0) ? ("T%d") : ("idle"), t);

    if (t == ctx->selected->id) {
      hline(area,
          GUI_MARGIN,
          (t+1 + 1) * get_line_height(ctx, area->h) - GUI_MARGIN,
          GUI_MARGIN + text_length(font, "  "),
          TEXT_COL);
    }
  }
}

/* Fills the trace area with all the info */
static void disp_trace(struct guictx *ctx, BITMAP *area) {
  int i, t;
  const struct trace *trace;
  int lh;       /* line height */
  long time_end;
  const struct trace_evt *evt, *prev_evt;
  long evt_time, prev_evt_time;
  struct timespec now;

  trace = &ctx->ts->trace;

  time_end = max_disp_time(ctx, area->w);
  lh = get_line_height(ctx, area->h);

  if (ctx->redraw) {
    printf_log(LOG_DEBUG, "Clearing trace area...\n");
    clear_to_color(area, BG_COL);

    for (t = -1; t < ctx->ts->tasks_count; t++) {
      disp_timeline(ctx, area, (t+1 + 1) * lh - GUI_MARGIN, false, false);
    }

    for (t = 0; t < ctx->ts->tasks_count; t++) {
      disp_at_dt(ctx, area, &ctx->ts->tasks[t], lh);
    }
  }

  if (ctx->redraw || (ctx->ts->activated && !ctx->ts->stopped)) {
    prev_evt = NULL;
    for (i = evt_preceding(ctx, ctx->disp_zero); i <= trace->len; i ++) {
      evt = &trace->events[i];
      evt_time = time_diff_ms(&evt->time, &ctx->ts->t0);

      assert(evt->valid || i == trace->len); /* not valid implies current */

      if (prev_evt != NULL && evt_time >= ctx->disp_zero) {
        disp_evt(ctx, area, prev_evt, prev_evt_time, evt_time, lh);
      }

      if (evt_time > time_end) break;

      if (i == trace->len && evt->valid && !ctx->ts->stopped) {  /* current */
        clock_gettime(CLOCK_MONOTONIC, &now);
        disp_evt(ctx,area, evt, evt_time, time_diff_ms(&now, &ctx->ts->t0), lh);
      }

      prev_evt = evt;
      prev_evt_time = evt_time;
    }
  }
}


/****** CPU LOAD ******/

static void disp_load_axes(struct guictx *ctx, BITMAP *area) {
  /* label */
  textprintf_ex(area, font,
      GUI_MARGIN, LOAD_PLOT_H / 2 + text_height(font) / 2,
      TEXT_COL, -1, "load");

  /* time axis */
  disp_timeline(ctx, area, area->h - GUI_MARGIN, false, true);

  /* y axis */
  rectfill(area,
      LINESTART_X - 2, area->h - GUI_MARGIN + 1,
      LINESTART_X - 1, area->h - GUI_MARGIN - LOAD_PLOT_H - GUI_MARGIN,
      TEXT_COL);

  /* y axis ticks */
  hline(area, LINESTART_X - 1 - TICK_LEN,
      area->h - GUI_MARGIN - 1,
      LINESTART_X - 1, TEXT_COL);
  hline(area, LINESTART_X - 1 - TICK_LEN,
      area->h - GUI_MARGIN - LOAD_PLOT_H - 1,
      LINESTART_X - 1, TEXT_COL);

  textprintf_right_ex(area, font,
      LINESTART_X - TICK_LEN - GUI_MARGIN,
      area->h - GUI_MARGIN - text_height(font)/2,
      TEXT_COL, -1, "0");
  textprintf_right_ex(area, font,
      LINESTART_X - TICK_LEN - GUI_MARGIN,
      area->h - GUI_MARGIN - text_height(font)/2 - LOAD_PLOT_H,
      TEXT_COL, -1, "1");
}

static int color_for_load(double load) {
  if (load < .5) return CPULOAD_OK_COL;
  if (load > .9) return CPULOAD_BAD_COL;
  return CPULOAD_AVG_COL;
}

static double get_load(struct guictx *ctx, long time_ms) {
  int i;
  const struct trace *trace;
  const struct trace_evt *evt, *prev_evt;
  long evt_time, prev_evt_time;
  struct timespec now;
  long now_ms;
  long start_time, tot_idle_time;

  trace = &ctx->ts->trace;
  start_time = time_ms - ctx->cpuload_window;

  if (trace->len <= 1  ||  start_time < 0  ||  time_ms > time_limit(ctx))
    return NAN;

  clock_gettime(CLOCK_MONOTONIC, &now);
  now_ms = time_diff_ms(&now, &ctx->ts->t0);

  if (time_ms > now_ms)
    return NAN;

  tot_idle_time = 0;
  prev_evt = NULL;
  for (i = evt_preceding(ctx, start_time); i <= trace->len; i ++) {
      evt = &trace->events[i];
      evt_time = time_diff_ms(&evt->time, &ctx->ts->t0);

      assert(evt->valid || i == trace->len); /* not valid implies current */

      if (prev_evt != NULL && evt->valid && prev_evt->task == -1) {
        assert(evt_time >= start_time);
        tot_idle_time +=
          MIN(evt_time, time_ms) - MAX(prev_evt_time, start_time);
      }

      if (evt_time > time_ms) break;

      if (i == trace->len && evt->valid && !ctx->ts->stopped && evt->task == -1)
      {
        tot_idle_time += (time_ms - MAX(evt_time, start_time));
      }

      prev_evt = evt;
      prev_evt_time = evt_time;
  }

  assert(0 <= tot_idle_time);
  if (tot_idle_time > ctx->cpuload_window)
    printf_log(LOG_ERROR,
        "idle: %d, window: %d\n", tot_idle_time, ctx->cpuload_window);
  assert(tot_idle_time <= ctx->cpuload_window);

  return 1.0 - (double)tot_idle_time / (double) ctx->cpuload_window;
}

static void disp_load(struct guictx *ctx, BITMAP *area) {
  int px;
  int plot_width;
  double cpuload;

  plot_width = area->w - LINESTART_X - LINEEND_X_ROFF;

  if (ctx->redraw) {
    clear_to_color(area, BG_COL);
    disp_load_axes(ctx, area);
  }

  if (ctx->redraw || (ctx->ts->activated && !ctx->ts->stopped)) {
    for (px = 0; px < plot_width; px ++) {
      cpuload = get_load(ctx, px_to_time(ctx, plot_width, px));
      if (! isnan(cpuload)) {
        vline(area, 
            px + LINESTART_X,
            area->h - GUI_MARGIN - 1 - (cpuload * LOAD_PLOT_H),
            area->h - GUI_MARGIN - 1,
            makecol(30, 30, 30));
        putpixel(area,
            px + LINESTART_X,
            area->h - GUI_MARGIN - 1 - (cpuload * LOAD_PLOT_H),
            color_for_load(cpuload));
      }
    }
  }
}


/****** EXPOSED API  ******/

void display_trace(struct guictx *ctx, BITMAP *area) {
  BITMAP *timeline_area, *trace_area, *lineheadings_area, *load_area;

  timeline_area = create_sub_bitmap(area,
      0, area->h - TIMELINE_H, area->w, TIMELINE_H);

  lineheadings_area = create_sub_bitmap(area,
      0, 0, LINESTART_X, area->h - TIMELINE_H - LOAD_H);

  load_area = create_sub_bitmap(area,
      0, area->h - LOAD_H - TIMELINE_H, area->w, LOAD_H);

  trace_area = create_sub_bitmap(area,
      LINESTART_X, 0,
      area->w - LINESTART_X - LINEEND_X_ROFF, area->h - TIMELINE_H - LOAD_H);

  if (ctx->redraw) {
    clear_to_color(timeline_area, BG_COL);
    disp_timeline(ctx, timeline_area, TIMELINE_Y, true, true);
  }
  disp_load(ctx, load_area);
  disp_headings(ctx, lineheadings_area);
  disp_trace(ctx, trace_area);

  destroy_bitmap(load_area);
  destroy_bitmap(timeline_area);
  destroy_bitmap(lineheadings_area);
  destroy_bitmap(trace_area);
}
