scheduletrace
=============

Scheduletrace is a utility to graphically display the scheduling of a set of ad-hoc periodic tasks running on a Linux system.
Its purpose is mainly didactic. Should you happen to find it useful in any other context, I'd be glad to be informed.

Written by Davide Kirchner for the _Real-Time systems_ course held by
professor Giorgio Buttazzo at TeCIP / Scuola Superiore Sant'Anna
di Studi Universitari e Perfezionamento (Pisa, Italy).

Project requirements:

    Visualize the schedule of a task set (with D ≤ T). Each task displays its execution.
    Time scale must be variable by the user. The execution of the main function has to be also
    visualized as idle time. Also visualize periodic activation times and absolute deadlines as longer
    vertical bars with different colors. Visualize critical sections protected by different semaphores
    using different colors. Build a task set in such a way a priority inversion occurs. Then run the
    same example using Priority Inheritance and Priority Ceiling to show that priority inversion
    disappears. Also visualize the instantaneous workload as a function of time.

Other than in this file, a description of the software is available in LaTeX _and_ pre-compiled in PDF in the `docs` folder.

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

A task is (not strictly formally) described as follows:

    TASK    ::=  T=<PERIOD>,D=<DEADLINE>,pr=<PRIORITY>,ph=<PHASE>[<SECTION>*]
    SECTION ::=  (R<RESOURCE>,<OP_COUNT>)

- `PERIOD`: Task period in _ms_
- `DEADLINE`: Relative deadline in _ms_
- `PRIORITY`: Scheduling priority. Use the range [3,99], because 1 and 2 are used internally
- `PHASE`: A positive offset for the first activation of the task.
- `RESOURCE`: Integer id of a resource. `R0` means no resource at all. From `R1` on, they are actual resources.
- `OP_COUNT`: Average number of operations in this section (while owning the corresponding resource)

Example:

    T=1000,D=500,pr=5,ph=100,[(R1,800000)(R0,200000)]

TODOs
-----
* Properly detect and show deadline misses
* Increased plot precision (currently 1ms, for some reasons I don't remember)
* Differently display "acquire/release" events?
* Priority-based task lock? Could avoid IDLE to wake up when it is not idle time at all...
* Load a saved trace

In case you're actually interested in one or more of these (or other) TODOs to become real, you are welcome to contact me. Unless there is some interest, this will most likely be abandoned shortly after taking my exam.

Authors
-------
Davide Kirchner  davide dot kirchner at yahoo dot it

Licence
-------
The core of this program is distrubuted under the Apache Licence, version 2.
A copy of the licence is shipped whith this code.

The `src/bsearch_left.c` file is distributed under the terms of the GNU General Public License, as published by the Free Software Foundation; version 2.

The `Makefile`, which wraps the SCons scripts, is released under the terms of the MIT licence (see into the file itself)
