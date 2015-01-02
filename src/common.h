#ifndef __COMMON_H__
#define __COMMON_H__

enum loglevel {LOG_ERROR=-1, LOG_WARNING=0, LOG_INFO=1, LOG_DEBUG=2};

typedef enum {false, true} bool;

void printf_log(enum loglevel level, const char *fmt, ...);

struct options {
  enum loglevel  verbosity;
  FILE*          logfile;
};

extern struct options options;

#endif
