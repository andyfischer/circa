

import os,sys
from glob import glob

DEBUG_BUILD = True

env = Environment()

# Get source files


source_files = list(glob("src/*.cpp"))
print "Source files = ", source_files
source_files.remove(os.path.join('src','test1.cpp'))

print "Platform =", sys.platform

if sys.platform == 'win32':
    env.Append(CPPFLAGS=['/EHsc'])

    if DEBUG_BUILD:
        env.Append(LINKFLAGS = ['/DEBUG'])  # enable debugging information
        env.Append(CCFLAGS = ['/MD'])        # MSVCRT.LIB & MSVCPRT.LIB
        env.Append(LINKFLAGS = ['/NODEFAULTLIB:"LIBCMT.LIB"'])
        env.Append(LINKFLAGS = ['/NODEFAULTLIB:"LIBCPMT.LIB"'])
        env.Append(CCFLAGS = ['/Od'])                  # Disable optimizations
        env.Append(CPPDEFINES = ["_DEBUG"])
        env.Append(CCFLAGS = ['/Z7'])
        env.Append(LINKFLAGS = ['/MACHINE:X86'])
        
        

else:
    if DEBUG_BUILD:
        env.Append(CPPFLAGS=['-ggdb'])

circa_lib = env.StaticLibrary('circa', source_files, CPPDEFINES=['DEBUG'])


env.Program('test1', ['src/test1.cpp'], LIBS = [circa_lib])
