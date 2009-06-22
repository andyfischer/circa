
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
    ROOT.Append(CPPFLAGS=['/EHsc /RTC1 /Wp64 /MTd /Z7 /Od /TP'.split()])
    ROOT.Append(LIBS = ['libcmtd'])
    ROOT.Append(LINKFLAGS=['/NODEFAULTLIB:libc.lib'])
    ROOT.Append(LINKFLAGS=['/NODEFAULTLIB:msvcrt.lib'])
    ROOT.Append(LINKFLAGS=['/NODEFAULTLIB:msvcrtd.lib'])
    ROOT.Append(LINKFLAGS=['/NODEFAULTLIB:libcd.lib'])
    ROOT.Append(LINKFLAGS=['/NODEFAULTLIB:libcmt.lib'])
    ROOT.Append(LINKFLAGS=['/SUBSYSTEM:CONSOLE /MACHINE:X86 /DEBUG'.split()])
    ROOT.Append(CPPDEFINES = ['WINDOWS'])

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

def download_file_from_the_internets(url, filename):
    print "Downloading "+url
    import urllib
    webFile = urllib.urlopen(url)
    localFile = open(filename, 'wb')
    localFile.write(webFile.read())
    webFile.close()
    localFile.close()

def unzip_file(filename, dir):
    print "Unzipping "+filename+" to "+dir
    import zipfile

    if not os.path.exists(dir): os.mkdir(dir)

    zf = zipfile.ZipFile(filename)
    for name in zf.namelist():
        path = os.path.join(dir, name)
        if name.endswith('/'):
            if not os.path.exists(path): os.mkdir(path)
        else:
            f = open(path, 'wb')
            f.write(zf.read(name))
            f.close()


SDL_ROOT = ROOT.Clone()

if POSIX:
    # import path so that we will find the correct sdl-config
    SDL_ROOT['ENV']['PATH'] = os.environ['PATH']
    SDL_ROOT.ParseConfig('sdl-config --cflags')
    SDL_ROOT.ParseConfig('sdl-config --libs')
    SDL_ROOT.Append(LIBS = ['SDL_gfx','SDL_image','SDL_ttf'])

    if MAC:
        SDL_ROOT['FRAMEWORKS'] = ['OpenGL']
    else:
        SDL_ROOT.Append(LIBS = ['OpenGL'])

if WINDOWS:
    SDL_ROOT.Append(LIBS=['opengl32.lib'])

    if not os.path.exists('SDL_deps'):
        # download SDL_deps.zip from github site
        download_file_from_the_internets('http://cloud.github.com/downloads/andyfischer/circa/SDL_deps.zip',              'SDL_deps.zip')

        # unzip it
        unzip_file('SDL_deps.zip', '.')

    SDL_ROOT.Append(CPPPATH=['#SDL_deps/SDL-1.2.13/include'])
    SDL_ROOT.Append(CPPPATH=['#SDL_deps/SDL_gfx-2.0.19'])
    SDL_ROOT.Append(CPPPATH=['#SDL_deps/SDL_image-1.2.7'])
    SDL_ROOT.Append(LIBS=['SDL_deps/SDL-1.2.13/lib/SDL.lib'])
    SDL_ROOT.Append(LIBS=['SDL_deps/SDL-1.2.13/lib/SDLmain.lib'])
    SDL_ROOT.Append(LIBS=['SDL_deps/SDL_gfx-2.0.19/VisualC/Release/SDL_gfx.lib'])
    SDL_ROOT.Append(LIBS=['SDL_deps/SDL_image-1.2.7/VisualC/Release/SDL_image.lib'])


SDL_ROOT.Append(CPPPATH=['#src'])
SDL_ROOT.Append(LIBS = [circa_staticlib])

Export('SDL_ROOT')

# Cuttlefish
SConscript('cuttlefish/build.scons')
