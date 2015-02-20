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
 * This file exposes the GUI entry point: its implementation lives in the gui/
 * subfolder.
 */

#ifndef __GUI_H__
#define __GUI_H__

#include "common.h"
#include "taskset.h"

#ifndef GUI_PERIOD
#define GUI_PERIOD 20
#endif

#define GUI_DEADLINE GUI_PERIOD

#ifndef GUI_THREAD_PRIORITY
#define GUI_THREAD_PRIORITY 2
#endif

#ifndef GUI_DEFAULT_W
#define GUI_DEFAULT_W 640
#endif

#ifndef GUI_DEFAULT_H
#define GUI_DEFAULT_H 480
#endif

#define GUI_MARGIN 5

/**
 * Run the allegro-based GUI. This function blocks unitl the GUI has started
 * and was closed.
 * The given taskset is expected to be initialized but not yet running.
 *
 * In case of failure, errors will be logged and exit() called.
 */
void gui_run(struct taskset *ts);

#define makegrey(x) makecol(x, x, x)

#define COL_RED         makecol(255,   0,   0)
#define COL_GREEN       makecol(  0, 255,   0)
#define COL_BLUE        makecol(  0,   0, 255)
#define COL_YELLOW      makecol(255, 255,   0)
#define COL_ORANGE      makecol(255, 100,   0)
#define COL_BLACK       makegrey(  0)
#define COL_WHITE       makegrey(255)

#define BG_COL          COL_BLACK
#define SPLITLINE_COL   makegrey(120)
#define HELP_TEXT_COL   makegrey( 90)
#define TEXT_COL        makegrey(180)
#define ACTIVATION_COL  COL_WHITE
#define DEADLINE_COL    COL_RED
#define CPULOAD_BG_COL  makegrey( 30)
#define CPULOAD_OK_COL  makecol(  0, 153,  51)
#define CPULOAD_AVG_COL makecol(230,  92,   0)
#define CPULOAD_BAD_COL makecol(230,   0,   0)

#endif
