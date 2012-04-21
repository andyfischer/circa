
# Build Targets #

The SCons build file has a recipe for this target:

Circa command-line:
  A command-line app with very few dependencies. This app can run scripts, run
  unit tests, print out debugging info, run a REPL, and other stuff. 

(More targets may be added in the future)

# Variants #

There are three build variants:

Test:
  As many assertions as possible are enabled, performance is not a concern. Includes
  such popular assertions as ca_test_assert() and the one that does a runtime type
  check on every single call.

  Binaries: circa_t

Debug:
  Some assertions are enabled, but performance-killing assertions are disabled. Symbols
  are enabled and optimizations are disabled. With this build you should be able to get
  decent performance, but also get decent diagnostics when something breaks.

  Binaries: circa_d

Release:
  Assertions disabled and optimizations enabled. This build runs fast and isn't as
  helpful when diagnosing bugs.
  
  Binaries: circa (no suffix)

## Building the command-line tool ##

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
in debug mode, run:

    scons build/circa_d

Available targets are:

 - Command line tool (release variant): build/circa
 - Command line tool (debug variant): build/circa_d
 - Command line tool (test variant): build/circa_t
 - Static library (releaase variant):
   - On Mac/Linux:
     build/libcirca.a
 - Static library (debug variant):
   - On Mac/Linux:
     build/libcirca_d.a
 - Static library (test variant):
   - On Mac/Linux:
     build/libcirca_t.a
