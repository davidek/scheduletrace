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


#define LINE_SPACING 5


void display_info(struct guictx *ctx, BITMAP *info_area) {
  //const char *line;
  int i;
  int ypos;
  int lineheight;
  struct task_params *task;

  task = ctx->selected;
  clear_to_color(info_area, BG_COL);

  lineheight = text_height(font) + LINE_SPACING;
  ypos = GUI_MARGIN + lineheight;  /* empty line */

  textprintf_ex(info_area, font, GUI_MARGIN, ypos, TEXT_COL, -1,
      "T%d selected:", task->id);
  ypos += lineheight;

  textprintf_ex(info_area, font, GUI_MARGIN, ypos, TEXT_COL, -1,
      " T %u ms, D %u ms", task->period, task->deadline);
  ypos += lineheight;

  textprintf_ex(info_area, font, GUI_MARGIN, ypos, TEXT_COL, -1,
      " priority %u, phase %u ms", task->priority, task->phase);
  ypos += lineheight;

  textprintf_ex(info_area, font, GUI_MARGIN, ypos, TEXT_COL, -1,
      " deadline misses: %u", task->dmiss);
  ypos += lineheight;

  textprintf_ex(info_area, font, GUI_MARGIN, ypos, TEXT_COL, -1,
      " Sections: %u", task->dmiss);
  ypos += lineheight;

  for (i = 0; i < task->sections_count; i++) {
    textprintf_ex(info_area, font, GUI_MARGIN + text_length(font, " "),
        ypos, TEXT_COL,
        get_resource_color(task->sections[i].res), " ");

    if (task->sections[i].res > 0)
      textprintf_ex(info_area, font, GUI_MARGIN, ypos, TEXT_COL, -1,
          "   R%u,%lu", task->sections[i].res, task->sections[i].avg);
    else
      textprintf_ex(info_area, font, GUI_MARGIN, ypos, TEXT_COL, -1,
          "   --,%lu", task->sections[i].avg);

    ypos += lineheight;
  }
}
