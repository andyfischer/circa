
import os, sys

from tools.Utils import *

def fatal(msg):
    print "fatal:",msg
    exit(1)

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

# Check for a Mac-like environment. On Macs, 'POSIX' is true, but there is
# a little bit of special behavior for using frameworks.
MAC = os.path.exists('/Library/Frameworks')

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
ROOT.SetOption('num_jobs', 2)
Export('ROOT')

# Build Circa library
CIRCA_ENV = ROOT.Clone()

CIRCA_ENV.BuildDir('build/src', 'src')
CIRCA_ENV.Append(CPPPATH = ['src'])

if not os.path.exists('build'):
    os.mkdir('build')

def write_text_file(path, contents):
    f = open(path, 'w')
    f.write(contents)
    f.write("\n")
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

circa_staticlib = CIRCA_ENV.StaticLibrary('build/bin/circa', BUILD_FILES)

circaBinary = CIRCA_ENV.Program('build/bin/circa', 'build/src/main.cpp', LIBS=[circa_staticlib])

CIRCA_ENV.Default(circaBinary)

########################### SDL-based targets ###############################

SDL_ROOT = ROOT.Clone()

if POSIX:
    # import path so that we will find the correct sdl-config
    SDL_ROOT['ENV']['PATH'] = os.environ['PATH']
    SDL_ROOT.ParseConfig('sdl-config --cflags')
    SDL_ROOT.ParseConfig('sdl-config --libs')
    SDL_ROOT.Append(LIBS = ['SDL_gfx'])

    if MAC:
        SDL_ROOT['FRAMEWORKS'] = ['OpenGL']

if WINDOWS:
    def look_for_required_dep_folder(folder):
        if not os.path.exists(folder):
            print "I couldn't find the folder "+folder+'/'

            # check for a common mistake with unzipping
            if os.path.exists('SDL_deps/SDL_deps'):
                print "However, I did find the folder SDL_deps/SDL_deps/, which means that"
                print "your unzipper may have created an extra folder. Try moving the nested"
                print "SDL_deps/ folder up one level, into the project directory."
            else:
                print "Make sure to download the file SDL_deps.zip from:"
                print "http://cloud.github.com/downloads/andyfischer/circa/SDL_deps.zip"
                print "and unzip it into this project folder."
            exit(1)

    look_for_required_dep_folder('SDL_deps')
    look_for_required_dep_folder('SDL_deps/SDL-1.2.13')
    look_for_required_dep_folder('SDL_deps/SDL_gfx-2.0.19')

    SDL_ROOT.Append(CPPPATH=['SDL_deps/SDL-1.2.13/include'])
    SDL_ROOT.Append(CPPPATH=['SDL_deps/SDL_gfx-2.0.19'])
    SDL_ROOT.Append(LIBS=['SDL_deps/SDL-1.2.13/lib/SDL.lib'])
    SDL_ROOT.Append(LIBS=['SDL_deps/SDL-1.2.13/lib/SDLmain.lib'])
    SDL_ROOT.Append(LIBS=['SDL_deps/SDL_gfx-2.0.19/VisualC/Release/SDL_gfx.lib'])


SDL_ROOT.Append(CPPPATH=['#/src'])
SDL_ROOT.Append(LIBS = [circa_staticlib])

Export('SDL_ROOT')

cuttlefish_bin = SDL_ROOT.Program('build/bin/cfsh', 'cuttlefish/main.cpp')
SDL_ROOT.Alias('cuttlefish', cuttlefish_bin)
SDL_ROOT.Alias('cfsh', cuttlefish_bin)

# 'ptc', an optional app
if os.path.exists('ptc'):
    SConscript('ptc/build.scons')
