

import os,sys
from glob import glob
env = Environment()

DEBUG_BUILD = True


# Get source files
source_files = list(glob("cpp/*.cpp"))

env.Append(CPPDEFINES = ["_DEBUG"])
env.Append(CPPDEFINES = ["DEBUG"])

# Python support
env.Append(CPPPATH = ["/System/Library/Frameworks/Python.framework/Versions/2.5/Headers"])

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
        
        
else:
    if DEBUG_BUILD:
        env.Append(CPPFLAGS=['-ggdb'])

circa_lib = env.SharedLibrary('circa', source_files)


env.Program('test1', ['cpp/Main.cpp'], LIBS = [circa_lib])
