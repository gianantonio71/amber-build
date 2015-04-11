# amber-build
All that is needed to build an Amber program under Linux.

This requires Linux. I personally use Lubuntu 14.10, but it should work just fine on any recent version of Linux. It also requires GNU g++ to be installed.

To compile your program, just replace the file main.ar in this repository with the one containing your own program and type make. If the compilation succeeds, you'll find three new files in the directory: *runme*, *generated.cpp* and *generated.h*. *runme* is the executable file for your program.

The repository for the compiler source code is here: https://github.com/gianantonio71/amber-compiler, and the one for the runtime files here: https://github.com/gianantonio71/amber-runtime
