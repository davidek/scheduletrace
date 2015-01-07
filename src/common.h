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

#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>

enum loglevel {LOG_ERROR=-1, LOG_WARNING=0, LOG_INFO=1, LOG_DEBUG=2};

typedef enum {false, true} bool;

void printf_log(enum loglevel level, const char *fmt, ...);

struct options {
  bool          help;           /* the --help flag */
  enum loglevel verbosity;
  FILE*         logfile;
  char*         infile_name;
  FILE*         infile;
};

extern struct options options;

#endif
