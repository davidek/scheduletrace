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
 * This module aims at simplifying error handling and logging troughout the
 * project.
 * It also provides a global `options` struct for run-time parameters and other
 * useful stuff.
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdbool.h>
#include <stdio.h>
#include <semaphore.h>
#include <assert.h>
#include <errno.h>

enum loglevel {LOG_ERROR=-1, LOG_WARNING=0, LOG_INFO=1, LOG_DEBUG=2};

/* typedef enum {false, true} bool; */ /* removed, now using stdbool */


/**
 * Similar to `assert` but _always_ executes the expression
 */
#define run_assert(expr) \
  do { if (! expr) assert(false); } while (0)


/**
 * A synchronized printf-like macro with an extra `loglevel` argument 
 * printf_log(enum loglevel level, const char *fmt, ...) 
 */
#define printf_log(level, ...) \
  do { \
    if (level <= options.verbosity) { \
      if (options.logfile_sync) run_assert(0==sem_wait(&options.logfile_sem)); \
      printf_log_nosync(level, 0, __VA_ARGS__); \
      if (options.logfile_sync) run_assert(0==sem_post(&options.logfile_sem)); \
    } \
  } while (0)


/**
 * Like printf_log, but also calls "perror" with the given error number
 * printf_log(enum loglevel level, int errnum, const char *fmt, ...) 
 */
#define printf_log_perror(level, e, ...) \
  do { \
    int errno_cache = e;  /* emulate call by value, so one can pass `errno` */ \
    if (level <= options.verbosity) { \
      if (options.logfile_sync) run_assert(0==sem_wait(&options.logfile_sem)); \
      printf_log_nosync(level, errno_cache, __VA_ARGS__); \
      if (options.logfile_sync) run_assert(0==sem_post(&options.logfile_sem)); \
    } \
  } while (0)


/* it is not advisable to call this directly: use the macros, instead */
void printf_log_nosync(enum loglevel level, int e, const char *fmt, ...);


/**
 * A struct for holding global settings and variables.
 */
struct options {
  bool          help;           /* the --help flag */
  enum loglevel verbosity;
  FILE*         logfile;
  bool          logfile_sync;   /* Whether to use atomic writes to logfile */
  sem_t         logfile_sem;    /* To implement atomic writes to logfile */
  char*         infile_name;
  FILE*         infile;
  /* bool          with_global_lock;       / * if --with-global-lock */
  sem_t         task_lock;
  unsigned long tick;
};

extern struct options options;

#endif
