#include <stdio.h>
#include <stdarg.h>

#include "common.h"


/* Global options instance */
struct options options;


/* A printf-like function for logging */
void printf_log(enum loglevel level, const char *fmt, ...) {
  va_list args;
  if (level <= options.verbosity) {
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

    if (level == LOG_ERROR) {
      fflush(options.logfile);
    }
  }
}

