
import os, sys, glob 

# generate setup_builtin_functions.cpp and register_all_tests.cpp
from scripts import generate_cpp_registration
generate_cpp_registration.do_builtin_functions(
    'src/builtin_functions',
    'src/setup_builtin_functions.cpp')
generate_cpp_registration.do_register_all_tests(
    'src/tests',
    'src/register_all_tests.cpp')

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

    if not releaseBuild:
        env.Append(CPPFLAGS=['-ggdb'])
        env.Append(CPPFLAGS=['-Wall'])

    return env

RELEASE_BUILD = False
SOURCE_DIRECTORY = "src"

ENV = create_mac_env(RELEASE_BUILD)
ENV.Append(CPPDEFINES = ["_DEBUG"])
ENV.Append(CPPDEFINES = ["DEBUG"])

ENV.BuildDir('build/src', 'src',duplicate=0)
ENV.Append(CPPPATH = ['src'])

if not os.path.exists('build'):
    os.mkdir('build')

# Precompiled header support
# ENV.Prepend(CPPPATH=['.'])
# ENV['Gch'] = ENV.Gch('common_headers.hpp')[0]
# ENV['GchSh'] = ENV.GchSh('src/common_headers.h')[0]

def write_text_file(path, contents):
    f = open(path, 'w')
    f.write(contents)
    f.close()

BUILD_FILES = []

def source_directory(dir, excludes=[]):
    for path in os.listdir(dir):
        full_path = os.path.join(dir,path)
        if not os.path.isfile(full_path): continue
        if path in excludes: continue
        if not path.endswith('.cpp'): continue

        global BUILD_FILES
        full_path = full_path.replace("src","build/src")
        BUILD_FILES.append(full_path)


def source_directory_into_one_cpp(dir, name):
    generated_cpp = []
    generated_filename = 'build/'+name+'.cpp'
    for path in os.listdir(dir):
        full_path = os.path.join(dir,path)
        if not os.path.isfile(full_path):
            continue

        generated_cpp.append("#include \"../" + full_path + "\"")

    generated_cpp = "\n".join(generated_cpp)
    write_text_file(generated_filename, generated_cpp)

    global BUILD_FILES
    BUILD_FILES.append(generated_filename)

source_directory('src', excludes=['test_program.cpp', 'main.cpp'])
source_directory_into_one_cpp('src/tests', 'all_tests')
source_directory_into_one_cpp('src/builtin_functions', 'all_builtin_functions')

# Find source files
#excludeFromLibrary = ['test_program.cpp', 'main.cpp']
#for (dirpath, dirnames, filenames) in os.walk('src'):
#    for file in filenames:
#        if file in excludeFromLibrary:
#            continue
#        fullpath = os.path.join(dirpath, file)
#        fullpath = fullpath.replace("src","build")
#
#        if fullpath.endswith('.cpp'):
#            BUILD_FILES.append(fullpath)


    
circa_so = ENV.StaticLibrary('lib/circa', BUILD_FILES)
circa_dl = ENV.SharedLibrary('bin/circa', BUILD_FILES)

circaBinary = ENV.Program('bin/circa', 'build/src/main.cpp', LIBS=[circa_so])

ENV.SetOption('num_jobs', 2)

ENV.Default(circaBinary)

