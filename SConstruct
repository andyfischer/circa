
import os, sys, glob 

from tools.Utils import *

# generate setup_builtin_functions.cpp and register_all_tests.cpp
from tools import generate_cpp_registration
generate_cpp_registration.do_builtin_functions(
    'src/builtin_functions',
    'src/setup_builtin_functions.cpp')
generate_cpp_registration.do_register_all_tests(
    'src/tests',
    'src/register_all_tests.cpp')

ENV = Environment(tools = ["default"], toolpath=".")

POSIX = os.name == 'posix'
WINDOWS = os.name == 'nt'

if POSIX:
    ENV.Append(CPPFLAGS=['-ggdb'])
    ENV.Append(CPPFLAGS=['-Wall'])

if WINDOWS:
    ENV.Append(CPPFLAGS=['/EHsc'])
    ENV.Append(CPPFLAGS=['/RTC1'])
    ENV.Append(CPPFLAGS=['/Wp64'])
    ENV.Append(CPPFLAGS=['/MTd'])
    ENV.Append(CPPFLAGS=['/Gm'])
    ENV.Append(CPPFLAGS=['/Zi'])
    ENV.Append(LIBS = ['libcmtd'])
    ENV.Append(LINKFLAGS=['/NODEFAULTLIB:libc.lib'])
    ENV.Append(LINKFLAGS=['/NODEFAULTLIB:msvcrt.lib'])
    ENV.Append(LINKFLAGS=['/NODEFAULTLIB:msvcrtd.lib'])
    ENV.Append(LINKFLAGS=['/NODEFAULTLIB:libcd.lib'])
    ENV.Append(LINKFLAGS=['/NODEFAULTLIB:libcmt.lib'])
    ENV.Append(LINKFLAGS=['/SUBSYSTEM:CONSOLE'])
    ENV.Append(LINKFLAGS=['/MACHINE:X86'])


ENV.Append(CPPDEFINES = ["_DEBUG"])
ENV.Append(CPPDEFINES = ["DEBUG"])

ENV.BuildDir('build/src', 'src')
ENV.Append(CPPPATH = ['src'])

if not os.path.exists('build'):
    os.mkdir('build')

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

source_directory('src', excludes=['main.cpp', 'src/main.cpp'])
source_directory_into_one_cpp('src/tests', 'all_tests')
source_directory_into_one_cpp('src/builtin_functions', 'all_builtin_functions')

circa_slib = ENV.StaticLibrary('lib/circa', BUILD_FILES)
# circa_so = ENV.SharedLibrary('lib/circa', BUILD_FILES)

circaBinary = ENV.Program('build/bin/circa', 'build/src/main.cpp', LIBS=[circa_slib])

ENV.SetOption('num_jobs', 2)
ENV.Default(circaBinary)

########################### SDL-based apps ###############################

SDL_ENV = Environment()

# import path so that we will find the correct sdl-config
SDL_ENV['ENV']['PATH'] = os.environ['PATH']

if POSIX:
    SDL_ENV.ParseConfig('sdl-config --cflags')
    SDL_ENV.ParseConfig('sdl-config --libs')

SDL_ENV.Append(LIBS = ['SDL_gfx'])
SDL_ENV.Append(CPPPATH=['src'])
SDL_ENV.Append(LIBPATH = "lib")
SDL_ENV.Append(LIBS = [circa_slib])

for app_name in read_file_as_lines('build_apps'):
    SDL_ENV.Program('build/bin/'+app_name, app_name+'/main.cpp')
    SDL_ENV.Alias(app_name, 'build/bin/'+app_name)
