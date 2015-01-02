#include <stdio.h>
#include <unistd.h>

#include <allegro.h>

#include "common.h"


static void options_init(int argc, char **argv) {
  options.verbosity = LOG_DEBUG;
  options.logfile = stderr;
}


static void graphics_init() {
  allegro_init();

  set_gfx_mode(GFX_AUTODETECT_WINDOWED, 640, 480, 0, 0);
}


int main(int argc, char **argv) {
  options_init(argc, argv);

  printf_log(LOG_INFO, "Starting scheduletrace...\n");

  graphics_init();

  sleep(3);

  printf_log(LOG_INFO, "Exiting scheduletrace...\n");
  return 0;
}
