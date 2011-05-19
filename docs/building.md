
This project has two primary build targets:

1. Circa command-line app
  - This tool can run scripts from a file or from an interactive REPL. It does not create
    a window and doesn't have support for graphics or sound. Has very few 3rd party dependencies.
2. Plastic
  - This application creates a window, and supports graphics and sound and other fancy things.
    Has many 3rd party dependencies.

# Building the command-line app #

 - First, install a C++ compiler as well as Python and SCons
 - Open a command prompt (sometimes called a Terminal) in the Circa directory, and then run the following commands:
   - python tools/prebuild.py
   - scons build/circa_t
   - build/circa_t (this will run Circa's unit tests)
  
 - This build has been tested on the following platforms:
   - Windows Vista (using Visual Studio 2005)
   - OSX 10.6 (with Xcode 3.2)
   - Ubuntu 10.04

# Building Plastic #

 - Install the following dependencies with your package manager. (Windows instructions are coming soon)
   - box2d
   - liblo
   - SDL-devel
   - SDL_gfx
   - SDL_image
   - SDL_ttf
 - Follow the above instructions to build the command-line app
 - Open a command prompt and run:
   - scons build/plas_r

 - This build has only been tested on OSX 10.6 (with Xcode 3.2)

# Build variants #

Each target has three build variants:
 - 'Test' build (ends in _t). Has every assertion enabled, including checks that have a major performance penalty. Runs slowly, but is good for checking for correctness.
 - 'Debug' build (ends in _d). Has some assertions enabled, and includes debugging symbols. Faster than a 'test' build, and can be used in a debugger.
 - 'Release' build (ends in _r). Assertions are disabled. This is the fastest build.

## Platform-specific instructions ##

Instructions are coming sooner or later.
