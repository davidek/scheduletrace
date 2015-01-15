scheduletrace
=============

A graphical tracer of scheduled tasks.

Based on pthreads and allegro

Written by Davide Kirchner for the Real-Time systems course held by
professor Giorgio Buttazzo at TeCIP / Scuola Superiore Sant'Anna
di Studi e Perfezionamento (Pisa, Italy).


Compiling
---------

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

