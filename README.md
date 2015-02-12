scheduletrace
=============

A graphical tracer of scheduled tasks.

Based on pthreads and allegro

Written by Davide Kirchner for the Real-Time systems course held by
professor Giorgio Buttazzo at TeCIP / Scuola Superiore Sant'Anna
di Studi e Perfezionamento (Pisa, Italy).


Compiling
---------

This is written in C99 and heavily relies on linux-only libraries. Thus, if
you're missing linux or a C99 compiler, install them or give up.

If you have SCons installed, simply cd to the project root and run

    scons

If, instead, you don't have SCons, there is a Makefile available that will
fetch a user-space installation and run it. Simply invoke it with:

    make

Running
-------
For a sample task set and default configuration, run it with

    ./scheduletrace -f ./taskset1

For a complete list of command-line arguments see

    ./scheduletrace --help

TODOs
-----
* Task/evt info on the right panel: (select task, show info of 1 task at a time)
* Properly detect and show deadline misses
* Double buffer trace plot?
* View speed independent of  "passata"?
  (perhaps not strictly needed once double buffering is implemented, but now it feels extremely silly)
* Increased plot precision (currently 1ms, for some reasons I don't remember)
* Differently display "acquire/release" events?
* New "Completed" event?
* Priority-based task lock?

Authors
-------
Davide Kirchner  davide dot kirchner at yahoo dot it
