
import os, sys, glob 

# generate setup_builtin_functions.cpp
from scripts import generate_setup_builtin_functions
generate_setup_builtin_functions.run('src/builtin_functions',
    'src/setup_builtin_functions.cpp')

def create_windows_env(releaseBuild):
    env = Environment(platform = 'nt')
    env.Append(CPPFLAGS=['/EHsc'])

    if not releaseBuild:
        env.Append(LINKFLAGS = ['/DEBUG'])  # enable debugging information
        env.Append(LINKFLAGS = ['/NODEFAULTLIB:"LIBCMT.LIB"'])
        env.Append(LINKFLAGS = ['/NODEFAULTLIB:"LIBCPMT.LIB"'])
        env.Append(LINKFLAGS = ['/MACHINE:X86'])
        env.Append(CCFLAGS = ['/MD'])        # MSVCRT.LIB & MSVCPRT.LIB
        env.Append(CCFLAGS = ['/Od'])        # Disable optimizations
        env.Append(CCFLAGS = ['/Z7'])

    return env
        
def create_mac_env(releaseBuild):
    env = Environment(tools = ["default"], toolpath=".")
    # Python support
    #env.Append(CPPPATH = ["/System/Library/Frameworks/Python.framework/Versions/2.5/Headers"])

    if not releaseBuild:
        env.Append(CPPFLAGS=['-ggdb'])
        env.Append(CPPFLAGS=['-Wall'])
        # env.Append(CPPFLAGS=['-bundle'])
        # env.Append(CPPFLAGS=['-flat_namespace'])

    return env

RELEASE_BUILD = False
SOURCE_DIRECTORY = "src"

env = create_mac_env(RELEASE_BUILD)
env.Append(CPPDEFINES = ["_DEBUG"])
env.Append(CPPDEFINES = ["DEBUG"])

env.BuildDir('build', 'src')
env.Append(CPPPATH = ['src'])

# Precompiled header support
# env.Prepend(CPPPATH=['.'])
# env['Gch'] = env.Gch('common_headers.hpp')[0]
# env['GchSh'] = env.GchSh('src/common_headers.h')[0]

# Find source files
excludeFromLibrary = ['test_program.cpp', 'main.cpp']
buildFiles = []
for (dirpath, dirnames, filenames) in os.walk('src'):
    for file in filenames:
        if file in excludeFromLibrary:
            continue
#if 'builtin_functions' in dirnames:
#dirnames.remove('builtin_functions')
        fullpath = os.path.join(dirpath, file)
        fullpath = fullpath.replace("src","build")

        if fullpath.endswith('.cpp'):
            buildFiles.append(fullpath)


    
circa_so = env.StaticLibrary('lib/circa', buildFiles)
circa_dl = env.SharedLibrary('bin/circa', buildFiles)

circaBinary = env.Program('bin/circa', 'build/main.cpp', LIBS=[circa_so])

env.SetOption('num_jobs', 2)

env.Default(circaBinary)

