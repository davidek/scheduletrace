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
 * This file exposes GUI-related functions which are implemented in the gui/
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
#define GUI_THREAD_PRIORITY 1
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


#define SPLITLINE_COL makecol(120, 120, 120)
#define HELP_TEXT_COL makecol( 90,  90,  90)

#define COL_RED makecol(255, 0, 0)
#define COL_GREEN makecol(0, 255, 0)
#define COL_BLUE makecol(0, 0, 255)

#endif
