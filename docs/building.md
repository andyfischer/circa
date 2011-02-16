
# Warning! These instructions have not been updated in a while, they are probably misleading! #

# Building the command-line app #

The command-line app can execute Circa scripts and has a REPL. However, this app
doesn't have any ability to create a window, draw graphics, etc. If you want that
stuff, then follow these instructions and then continue on to build 'Plastic', 
which has more instructions below.

## OSX ##

1. Install Xcode from Apple's site
2. Install Scons.
4. In Terminal, from the Circa project folder, enter the command: 'scons'

## Windows ##

1. Install Visual Studio (currently we have only tested against VS 2005)
2. Install Python 2.6 from www.python.org
3. Install SCons from here: www.scons.org
5. Open a command prompt, navigate to the Circa project folder, and run 'scons'

## Linux ##

1. Install GCC, Python, SCons
2. In a terminal, from the Circa project folder, run the command: 'scons'

# Building Plastic #

'Plastic' is the name for the app that creates a graphical window. It includes Circa
and depends on the SDL library.

## OSX ##

1. Follow the instructions above for building the command-line app
2. Install SDL with the gfx and image packages.
2.1 One way you can do this is to install MacPorts, then run this command:
     sudo port install SDL-devel SDL_gfx SDL_image
3. In Terminal, from the Circa project folder, enter the command: 'scons plastic'

## Windows ##

Warning, there is a known problem where the Microsoft CRT dll is not copied to the bin
folder, this will be fixed soon.

1. Follow the instructions above for building the command-line app
2. Run the script: tools/wrangle_deps.py
3. Open a command prompt in this folder and run 'scons plastic'

## Linux ##

Warning, this build target is untested and unsupported, so it probably won't work
without modification.

1. Follow the instructions above for building the command-line app
2. Install the following packages with your package manager:
     libsdl-dev, libsdl-image1.2-dev, libsdl-gfx1.2-dev, libsdl-ttf2.0-dev
3. In a terminal, from the Circa project folder, run the command: 'scons plastic'

