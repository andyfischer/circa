

import os,sys, glob
env = Environment()

DEBUG_BUILD = True
SOURCE_DIRECTORY = "cpp"

env.Append(CPPDEFINES = ["_DEBUG"])
env.Append(CPPDEFINES = ["DEBUG"])

if sys.platform == 'win32':
    env.Append(CPPFLAGS=['/EHsc'])

    if DEBUG_BUILD:
        env.Append(LINKFLAGS = ['/DEBUG'])  # enable debugging information
        env.Append(LINKFLAGS = ['/NODEFAULTLIB:"LIBCMT.LIB"'])
        env.Append(LINKFLAGS = ['/NODEFAULTLIB:"LIBCPMT.LIB"'])
        env.Append(LINKFLAGS = ['/MACHINE:X86'])
        env.Append(CCFLAGS = ['/MD'])        # MSVCRT.LIB & MSVCPRT.LIB
        env.Append(CCFLAGS = ['/Od'])        # Disable optimizations
        env.Append(CCFLAGS = ['/Z7'])
        
else: # Mac build
    # Python support
    env.Append(CPPPATH = ["/System/Library/Frameworks/Python.framework/Versions/2.5/Headers"])
    if DEBUG_BUILD:
        env.Append(CPPFLAGS=['-ggdb'])



def getDirectories(basepath):
    for filename in os.listdir(basepath):
        path = os.path.join(basepath, filename)
        if os.path.isdir(path):
            yield (filename, path)

# Loop through source directory and build everything

import platform
print platform.python_version()

for (dir_name, dir_path) in getDirectories(SOURCE_DIRECTORY):
    
    # Get all the source files in this directory
    source_files = list(glob.glob(dir_path + '/*.cpp'))

    # If this directory contains a "Main.cpp" then consider it a program
    is_program = False

    for b in map(lambda f: f.endswith("Main.cpp"), source_files): # no 'any' function
        if b: is_program = True

    if is_program:
        prog = env.Program(dir_name, source_files)
    else:
        lib = env.SharedLibrary(dir_name, source_files)
    

