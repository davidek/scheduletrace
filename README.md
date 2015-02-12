scheduletrace
=============

A graphical tracer of scheduled tasks.

Based on pthreads and allegro.

Written by Davide Kirchner for the Real-Time systems course held by
professor Giorgio Buttazzo at TeCIP / Scuola Superiore Sant'Anna
di Studi e Perfezionamento (Pisa, Italy).

Software requirements:

    Visualize the schedule of a task set (with D ≤ T). Each task displays its execution.
    Time scale must be variable by the user. The execution of the main function has to be also
    visualized as idle time. Also visualize periodic activation times and absolute deadlines as longer
    vertical bars with different colors. Visualize critical sections protected by different semaphores
    using different colors. Build a task set in such a way a priority inversion occurs. Then run the
    same example using Priority Inheritance and Priority Ceiling to show that priority inversion
    disappears. Also visualize the instantaneous workload as a function of time.

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

Taskset format
--------------

A taskset file is a simple file describing 1 task per line. Empty lines and lines starting with `#` are ignored.

A task is described as follows:

    TASK    ::=  T=<PERIOD>,D=<DEADLINE>,pr=<PRIORITY>,[<SECTION>*]
    SECTION ::=  (R<RESOURCE>,avg=<OP_COUNT>,dev=<OP_COUNT_DEV>)

- `PERIOD`: Task period in _ms_
- `DEADLINE`: Relative deadline in _ms_
- `PRIORITY`: Scheduling priority. Use the range [3,99], because 1 and 2 are used internally
- `RESOURCE`: Integer id of a resource. `R0` means no resource at all. From `R1` on, they are actual resources.
- `OP_COUNT`: Average number of operations in this section (while owning the corresponding resource)
- `OP_COUNT_DEV`: Standard deviation of `OP_COUNT`. Currently ignored.

TODOs
-----
* Task/evt info on the right panel: (select task, show info of 1 task at a time)
* "Also visualize the instantaneous workload as a function of time"
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
