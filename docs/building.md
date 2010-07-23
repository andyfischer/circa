
# OSX instructions #

1. Install Xcode from Apple's site
2. Install MacPorts from www.macports.org
3. In Terminal, use this command to install SDL and Scons:
     sudo port install SCons SDL-devel SDL_gfx SDL_image
4. In Terminal, from the Circa project folder, enter the command: 'scons all'

# WINDOWS instructions #

1. Install Visual Studio. (this build has been tested with VC++ 2005)
2. Install Python 2.6 from www.python.org
3  Install SCons from here: www.scons.org
4. Open a command prompt in this folder and run 'scons all'

Note, currently we have only tested building with VS 2005

# LINUX #

1. Install the following packages with your package manager:
     SCons, libsdl-dev, libsdl-image1.2-dev, libsdl-gfx1.2-dev, libsdl-ttf2.0-dev
2. In a terminal, run the command: 'scons all'

Warning, the Linux build has received very little testing and will likely not work
