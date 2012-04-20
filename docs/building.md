
Here are instructions for building circa.lib and 'circa' the command-line tool.

# Requirements #

You'll need:
 - A C++ compiler. Tested on GCC on Mac/Linux, and Visual Studio 2008 on Windows.
 - Python
 - Scons

# Environment #

Set the environment variable CIRCA_HOME to the root directory of the project.

# Building #

Open a command prompt in the project's root directory.

For the first build, and every time a source file is added or removed, you'll need to run
the prebuild.py script. Example:

    python tools/prebuild.py

Then, use "scons" to build the target that you want. For example, to build the command-line tool
in test mode, run:

    scons build/circa_t

Available targets are:

 - Command line tool (release variant): build/circa
 - Command line tool (debug variant): build/circa_d
 - Command line tool (test variant): build/circa_t
 - Static library (releaase variant): build/circa.lib
 - Static library (debug variant): build/circa_d.lib
 - Static library (test variant): build/circa_t.lib

# Variants #

 - Test (ends in _t). Every assertion in the world is enabled. Unit tests are also included. The slowest option, but is good for checking for correctness.
 - Debug (ends in _d). Some assertions enabled, and optimizations disabled. No unit tests.
 - Release (no suffix). Assertions are disabled and optimizations enabled. No unit tests.
