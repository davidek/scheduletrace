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
 * This file holds the implementation for the command-line interface.
 *
 * The main function sets up the global `options` parsing the command line
 * and triggers all the required threads to start, using their respective APIs.
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <semaphore.h>
#include <sched.h>
#include <pthread.h>

#include "common.h"
#include "taskset.h"
#include "gui.h"


/** Print the command line help */
void help(const char* cmd_name) {
  printf("\
Usage: %s [OPTION]...\n\
Runs some threads and displays their schedule.\n\
\n\
Mandatory arguments to long options are mandatory for short options too.\n\
  -h, --help            Display this help and exit.\n\
\n\
  -v, --verbose         Verbose output. Useful for debugging purposes.\n\
  -q, --quiet           Quiet mode: will only log warnings and fatal errors.\n\
  -g, --no-gui          Don't start the GUI.\n\
  -f, --taskfile=FILE   Read task definition from FILE (default or \"-\": stdin).\n\
  -t, --tracefile=FILE  Output trace to FILE (default or \"-\": stdout).\n\
      --no-trace        Disable output of the trace.\n\
      --trace-flush     Flush the output after writing each trace event.\n\
      --no-log-sync     Disable synchronization of logging statements \n\
                        (otherways enabled by default).\n\
                        When disabled, the output might turn into a mess.\n\
\n\
  -W, --width=NUM       Set window width to NUM.\n\
  -H, --height=NUM      Set window height to NUM.\n\
\n\
Controlling behaviour:\n\
  -p  --protocol=PROTO  Use the specified protocol for the mutex variables\n\
                        that emulate shared resources.\n\
                        PROTO can be NONE (default), INHERIT, or PROTECT.\n\
                        See PTHREAD_MUTEXATTR_GETPROTOCOL(3P) for details.\n\
      --no-affinity     Don't set the CPU affinity of the running tasks.\n\
                        By default, tasks are forced to run on a single\n\
                        processor (the first available is chosen): this flag\n\
                        inhibits this constraint. Beware that this may result\n\
                        in drastically different behaviours on multi-processor\n\
                        environments.\n\
      --idle-yield      If set, the idle job will invoke pthread_yield() at\n\
                        every operation.\n\
      --idle-sleep      If set, the idle job will invoke clock_nanosleep() with\n\
                        a 1-ns sleep time at every operation.\n\
\n\
", cmd_name);

/*
  -r, --run=SEC         Start taskset immediately, then stop it after (approxi-\n\
                        matively) SEC seconds.\n\
                        Expecially useful when --no-gui is selected.\n\
\n\
*/


}


/** Print a "see help" message */
void see_help(const char* cmd_name) {
  fprintf(stderr,"Try '%s --help' for more information.\n", cmd_name);
}

#define NO_LOG_SYNC     257
#define NO_TRACE        258
#define NO_AFFINITY     259
#define TRACE_FLUSH     260
#define IDLE_YIELD      261
#define IDLE_SLEEP      262

/** Populate options struct, parsing the command line arguments. */
void options_init(int argc, char **argv) {
  int s;        /* return value of library functions */
  int c;        /* the parsed option in the parsing loop */
  char short_options[] = "hvqgf:t:W:H:p:";
  struct option long_options[] = {
    {"help", no_argument, NULL, 'h'},
    {"verbose", no_argument, NULL, 'v' },
    {"quiet", no_argument, NULL, 'q' },
    {"no-gui", no_argument, NULL, 'g' },
    {"taskfile", required_argument, NULL, 'f'},
    {"tracefile", required_argument, NULL, 't'},
    {"trace-flush", no_argument, NULL, TRACE_FLUSH},
    {"no-trace", no_argument, NULL, NO_TRACE},
    {"no-log-sync", no_argument, NULL, NO_LOG_SYNC},
    {"width", required_argument, NULL, 'W'},
    {"height", required_argument, NULL, 'H'},
    {"protocol", required_argument, NULL, 'p'},
    {"no-affinity", no_argument, NULL, NO_AFFINITY},
    {"idle-yield", no_argument, NULL, IDLE_YIELD},
    {"idle-sleep", no_argument, NULL, IDLE_SLEEP},
    {NULL, 0, NULL, 0}
  };

  /* Initialize option to defaults */
  options.help = false;
  options.verbosity = LOG_INFO;
  options.with_gui = true;
  options.logfile = stderr;
  options.logfile_sync = true;
  options.taskfile_name = "-";
  options.taskfile = stdin;
  options.tracefile_name = "-";
  options.tracefile = stdout;
  options.tracefile_flush = false;
  options.gui_w = GUI_DEFAULT_W;
  options.gui_h = GUI_DEFAULT_H;
  options.mutex_protocol = PTHREAD_PRIO_NONE;
  options.with_affinity = true;
  CPU_ZERO(&options.task_cpuset);
  options.idle_rt_sched = true;

  /* Parse command line */
  while (true) {
    int option_index = 0;
    c = getopt_long(argc, argv, short_options, long_options, &option_index);
    if (c == -1)
      break;
    switch (c) {
      case 'h':
        options.help = true;
        break;
      case 'v':
        options.verbosity = LOG_DEBUG;
        break;
      case 'q':
        options.verbosity = LOG_WARNING;
        break;
      case 'g':
        options.with_gui = false;
        break;
      case 'f':
        assert(optarg != NULL);
        options.taskfile_name = optarg;
        break;
      case 't':
        assert(optarg != NULL);
        options.tracefile_name = optarg;
        break;
      case NO_TRACE:
        options.tracefile_name = NULL;
        options.tracefile = NULL;
        break;
      case TRACE_FLUSH:
        options.tracefile_flush = true;
        break;
      case NO_LOG_SYNC:
        options.logfile_sync = false;
        break;
      case 'W':
        assert(optarg != NULL);
        s = sscanf(optarg, "%d", &options.gui_w);
        if (s < 1) {
          printf("Invalid value for width (not an integer): %s", optarg);
          abort();
        }
        break;
      case 'H':
        assert(optarg != NULL);
        s = sscanf(optarg, "%d", &options.gui_h);
        if (s < 1) {
          printf("Invalid value for height (not an integer): %s\n", optarg);
          abort();
        }
        break;
      case 'p':
        assert(optarg != NULL);
        if (strcasecmp(optarg, "NONE") == 0)
          options.mutex_protocol = PTHREAD_PRIO_NONE;
        else if (strcasecmp(optarg, "INHERIT") == 0)
          options.mutex_protocol = PTHREAD_PRIO_INHERIT;
        else if (strcasecmp(optarg, "PROTECT") == 0)
          options.mutex_protocol = PTHREAD_PRIO_PROTECT;
        else {
          printf("Invalid value for protocol: %s\n", optarg);
          see_help(argv[0]);
          abort();
        }
        break;
      case NO_AFFINITY:
        options.with_affinity = false;
        break;
      case IDLE_YIELD:
        options.idle_yield = true;
        break;
      case IDLE_SLEEP:
        options.idle_sleep = true;
        break;
      case '?':
        /* getopt_long already printed an error message. */
        see_help(argv[0]);
        exit(1);
        break;
      default:
        abort();
    }
  }

  if (options.help) {
    return;  /* skip other initialization procedures */
  }

  if (options.logfile_sync) {
    s = sem_init(&options.logfile_sem, 0, 1);
    if (s < 0) {
      perror("Error initializing logging semaphore");
      exit(1);
    }
  }
  /* From this point on, printf_log can be used */
  if (! options.logfile_sync) {
    printf_log(LOG_INFO, "Console output synchronizatio disabled: output could"
        " become messy.\n");
  }

  if (strcmp(options.taskfile_name, "-") != 0) {
    options.taskfile = fopen(options.taskfile_name, "r");
    if (options.taskfile == NULL) {
      printf_log_perror(LOG_ERROR, errno, "Error while opening file \"%s\": ",
          options.taskfile_name);
      exit(1);
    }
  }

  if (options.tracefile_name != NULL
      && strcmp(options.tracefile_name, "-") != 0)
  {
    options.tracefile = fopen(options.tracefile_name, "a");
    if (options.tracefile == NULL) {
      printf_log_perror(LOG_ERROR, errno, "Error while opening file \"%s\": ",
          options.tracefile_name);
      exit(1);
    }
  }
  if (options.tracefile != NULL) {
    s = fprintf(options.tracefile, "== Beginning of scheduletrace TRACE ==\n");
    assert(s >= 0);
  }

  if (options.with_affinity) {
    int cpuid;
    cpu_set_t cpuset;

    s = sched_getaffinity(getpid(), sizeof(cpu_set_t), &cpuset);
    if (s < 0) {
      printf_log_perror(LOG_ERROR, errno,"sched_getaffinity returned error: ");
      exit(1);
    }
    printf_log(LOG_DEBUG, "Process has %d CPUs available:\n",
        CPU_COUNT(&cpuset), cpuset);
    for (cpuid = 0; cpuid < CPU_SETSIZE; cpuid++) {
      if (CPU_ISSET(cpuid, &cpuset))
        printf_log(LOG_DEBUG, "  CPU %d\n", cpuid);
    }
    for (cpuid = 0; cpuid < CPU_SETSIZE; cpuid++) {
      if (CPU_ISSET(cpuid, &cpuset)) break;
    }
    CPU_SET(cpuid, &options.task_cpuset);
    assert(CPU_COUNT(&options.task_cpuset) == 1);
    printf_log(LOG_DEBUG, "Tasks will run on CPU %d\n", cpuid);
  }
}


int main(int argc, char **argv) {
  struct taskset ts;

  options_init(argc, argv);

  if (options.help) {
    help(argv[0]);
    exit(0);
  }

  printf_log(LOG_INFO, "Starting scheduletrace...\n");

  if (strcmp(options.taskfile_name, "-") == 0) {
    printf_log(LOG_INFO, "Will read taskset description from STDIN.\n");
  }

  taskset_init_file(&ts);
  taskset_print(&ts);
  taskset_create(&ts);

  printf_log(LOG_INFO, "Taskset successfully initialized!\n");

  if (options.with_gui) {
    printf_log(LOG_INFO, "Starting GUI\n");
    gui_run(&ts);
  }
  else {
    printf_log(LOG_INFO, "GUI _not_ started upon user request.\n");

    taskset_activate(&ts);

    sleep(1);
    printf_log(LOG_INFO, "Quitting tasks!\n");
    taskset_quit(&ts);
    taskset_join(&ts);
  }

  printf_log(LOG_INFO, "Exiting scheduletrace.\n");
  exit(0);
  //pthread_exit(NULL);
}
