
import os, sys

from tools.Utils import *

# generate setup_builtin_functions.cpp and register_all_tests.cpp
from tools import generate_cpp_registration
generate_cpp_registration.do_builtin_functions(
    'src/builtin_functions',
    'src/setup_builtin_functions.cpp')
generate_cpp_registration.do_register_all_tests(
    'src/tests',
    'src/register_all_tests.cpp')

ROOT = Environment(tools = ["default"], toolpath=".")

POSIX = os.name == 'posix'
WINDOWS = os.name == 'nt'

if POSIX:
    ROOT.Append(CPPFLAGS=['-ggdb'])
    ROOT.Append(CPPFLAGS=['-Wall'])

if WINDOWS:
    ROOT.Append(CPPFLAGS=['/EHsc /RTC1 /Wp64 /MTd /Gm /Zi'.split()])
    ROOT.Append(LIBS = ['libcmtd'])
    ROOT.Append(LINKFLAGS=['/NODEFAULTLIB:libc.lib'])
    ROOT.Append(LINKFLAGS=['/NODEFAULTLIB:msvcrt.lib'])
    ROOT.Append(LINKFLAGS=['/NODEFAULTLIB:msvcrtd.lib'])
    ROOT.Append(LINKFLAGS=['/NODEFAULTLIB:libcd.lib'])
    ROOT.Append(LINKFLAGS=['/NODEFAULTLIB:libcmt.lib'])
    ROOT.Append(LINKFLAGS=['/SUBSYSTEM:CONSOLE /MACHINE:X86'.split()])

ROOT.Append(CPPDEFINES = ["_DEBUG"])
ROOT.Append(CPPDEFINES = ["DEBUG"])

ENV = ROOT.Clone()

ENV.BuildDir('build/src', 'src')
ENV.Append(CPPPATH = ['src'])

if not os.path.exists('build'):
    os.mkdir('build')

def write_text_file(path, contents):
    f = open(path, 'w')
    f.write(contents)
    f.close()

path_join = os.path.join

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

circa_staticlib = ENV.StaticLibrary('build/bin/circa', BUILD_FILES)

circaBinary = ENV.Program('build/bin/circa', 'build/src/main.cpp', LIBS=[circa_staticlib])

ENV.SetOption('num_jobs', 2)
ENV.Default(circaBinary)

########################### SDL-based apps ###############################

SDL_ENV = ROOT.Clone()

if POSIX:
    # import path so that we will find the correct sdl-config
    SDL_ENV['ENV']['PATH'] = os.environ['PATH']
    SDL_ENV.ParseConfig('sdl-config --cflags')
    SDL_ENV.ParseConfig('sdl-config --libs')
    SDL_ENV.Append(LIBS = ['SDL_gfx'])

elif WINDOWS:
    SDL_ENV.Append(CPPPATH=['SDL/include'])
    SDL_ENV.Append(CPPPATH=['SDL_gfx'])
    SDL_ENV.Append(LIBS=['SDL/lib/SDL.lib'])
    SDL_ENV.Append(LIBS=['SDL/lib/SDLmain.lib'])
    SDL_ENV.Append(LIBS=['SDL_gfx/VisualC/Release/SDL_gfx.lib'])


SDL_ENV.Append(CPPPATH=['src'])
SDL_ENV.Append(LIBS = [circa_staticlib])

for app_name in read_file_as_lines('build_apps'):
    prog = SDL_ENV.Program(path_join(app_name,'bin'), path_join(app_name,'main.cpp'))
    SDL_ENV.Alias(app_name, prog)
