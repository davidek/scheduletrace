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
#include <unistd.h>
#include <getopt.h>
#include <assert.h>

#include <allegro.h>

#include "common.h"
#include "task.h"
#include "taskset.h"
#include "observer.h"


/* Print the command line help */
void help(const char* cmd_name) {
  printf("\
Usage: %s [options]\n\
Runs some threads and displays their schedule.\n\
\n\
  -h, --help          Display this help and exit\n\
\n\
  -v, --verbose       Verbose output. Useful for debugging purposes.\n\
  -q, --quiet         Quiet mode: will only log warnings and fatal errors.\n\
\n\
  -f, --file=FILE     Read task definition from FILE rather than from stdin.\n\
", cmd_name);
}


/* Print a "see help" message */
void see_help(const char* cmd_name) {
  fprintf(stderr,"Try '%s --help' for more information.\n", cmd_name);
}


/* Populate options struct, parsing the command line arguments. */
void options_init(int argc, char **argv) {
  int c;  /* the parsed option in the parsing loop */
  struct option long_options[] = {
    {"help", no_argument, NULL, 'h'},
    {"verbose", no_argument, NULL, 'v' },
    {"quiet", no_argument, NULL, 'q' },
    {"file", required_argument, NULL, 'f'},
    {NULL, 0, NULL, 0}
  };

  /* Initialize option to defaults */
  options.help = false;
  options.verbosity = LOG_INFO;
  options.logfile = stderr;
  options.infile_name = "-";
  options.infile = stdin;

  /* Parse command line */
  while (true) {
    int option_index = 0;
    c = getopt_long(argc, argv, "hvqf:", long_options, &option_index);
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
      case 'f':
        assert(optarg != NULL);
        options.infile_name = optarg;
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

  if (strcmp(options.infile_name, "-") != 0) {
    options.infile = fopen(options.infile_name, "r");
    if (options.infile == NULL) {
      printf_log(LOG_ERROR, "Error while opening file \"%s\".\n",
          options.infile_name);
      exit(1);
    }
  }
}


/* Fully initialize the graphics library and start the GUI threads */
void graphics_init() {
  allegro_init();

  set_gfx_mode(GFX_AUTODETECT_WINDOWED, 640, 480, 0, 0);
}


int main(int argc, char **argv) {
  struct taskset ts;

  options_init(argc, argv);

  if (options.help) {
    help(argv[0]);
    exit(0);
  }

  printf_log(LOG_INFO, "Starting scheduletrace...\n");

  graphics_init();

  /*task_params_init(&params);
  params.period = 100;
  lonely_task = &params;*/

  taskset_init_file(&ts);
  taskset_print(&ts);
  
  printf_log(LOG_INFO, "Taskset successfully initialized!\n");

  observer_start(&ts);

  /*task_start(&params);*/

  taskset_start(&ts);
  
  sleep(1);
  printf_log(LOG_INFO, "Quitting tasks...\n");
  taskset_quit(&ts);
  sleep(1);

  printf_log(LOG_INFO, "Exiting scheduletrace.\n");
  exit(0);
}
