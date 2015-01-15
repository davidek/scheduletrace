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

#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>
#include <semaphore.h>

#include "common.h"


/* Global options instance */
struct options options;

/* A printf-like function for logging, to be called using the provided macros */
void printf_log_nosync(enum loglevel level, int e, const char *fmt, ...) {
  va_list args;
  char tname[16];

  if (level <= options.verbosity) {
    pthread_getname_np(pthread_self(), tname, 16);
    fprintf(options.logfile, "%6s: ", tname);

    switch(level) {
      case LOG_ERROR:
        fprintf(options.logfile, "Error: ");
        break;
      case LOG_WARNING:
        fprintf(options.logfile, "Warning: ");
        break;
      case LOG_INFO:
        fprintf(options.logfile, "Info: ");
        break;
      case LOG_DEBUG:
        fprintf(options.logfile, "Debug: ");
        break;
    }
    va_start(args, fmt);
    vfprintf(options.logfile, fmt, args);
    va_end(args);

    if (e > 0) {
        errno = e;
        perror("");
    }

    if (level == LOG_ERROR) {
      fflush(options.logfile);
    }
  }
}

