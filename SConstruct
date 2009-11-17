
import os, sys

from tools import SconsHelper

def fatal(msg):
    print "fatal:",msg
    exit(1)

POSIX = os.name == 'posix'
WINDOWS = os.name == 'nt'

# Check for a Mac-like environment. On Macs, 'POSIX' is true, but there is
# a little bit of special behavior for using frameworks.
MAC = os.path.exists('/Library/Frameworks')

DEBUG = Environment(tools = ["default"], toolpath=".")
RELEASE = Environment(tools = ["default"], toolpath=".")
COMMON = SconsHelper.EnvironmentSet(debug = DEBUG, release = RELEASE)

if not os.path.exists('build'):
    os.mkdir('build')

# Common build flags
if POSIX:
    COMMON.Append(CPPFLAGS=['-ggdb'])
    COMMON.Append(CPPFLAGS=['-Wall'])
    DEBUG.Append(CPPDEFINES = ["DEBUG"])
    RELEASE.Append(CPPFLAGS=['-O3'])
    COMMON.SetOption('num_jobs', 2)

if WINDOWS:
    COMMON.Append(CPPDEFINES = ['WINDOWS'])
    COMMON.Append(LINKFLAGS='/SUBSYSTEM:CONSOLE /MACHINE:X86'.split())
    DEBUG.Append(CPPFLAGS='/EHsc /W3 /MDd /Z7 /TP /Od'.split())
    DEBUG.Append(LINKFLAGS=['/NODEFAULTLIB:msvcrt.lib', '/DEBUG'])
    DEBUG.Append(CPPDEFINES = ["DEBUG", "_DEBUG"])
    RELEASE.Append(CPPFLAGS='/EHsc /W3 /MD /Z7 /O2 /TP'.split())
    RELEASE.Append(CPPDEFINES = ["NDEBUG"])

Export('WINDOWS')
Export('MAC')
Export('POSIX')

# Build static library
CIRCA_ENV = COMMON.Clone()
CIRCA_ENV.BuildDir('build/$buildname/src', 'src')
CIRCA_ENV.Append(CPPPATH = ['src'])

BUILD_FILES = []

from tools import prebuild
prebuild.main()

def list_source_files(dir):
    for path in os.listdir(dir):
        full_path = os.path.join(dir,path)
        if not os.path.isfile(full_path): continue
        if not path.endswith('.cpp'): continue
        yield path

library_source_files = (list(list_source_files('src')) + 
            ['generated/'+filename for filename in list_source_files('src/generated')])

library_source_files = filter(lambda f: f != 'main.cpp', library_source_files)

circa_static_libs = CIRCA_ENV.StaticLibrary('build/$buildname/circa',
    ['build/$buildname/src/'+filename for filename in library_source_files])

# Build command-line app (using debug env)
CIRCA_CL_ENV = CIRCA_ENV['debug']

cl_binary = CIRCA_CL_ENV.Program('build/bin/circa',
    'build/debug/src/main.cpp', LIBS=[circa_static_libs['debug']])

CIRCA_CL_ENV.Default(cl_binary)

########################### SDL-based targets ###############################


SDL_ROOT = RELEASE.Clone()

if POSIX:
    # import path so that we will find the correct sdl-config
    SDL_ROOT['ENV']['PATH'] = os.environ['PATH']
    try:
        SDL_ROOT.ParseConfig('sdl-config --cflags')
        SDL_ROOT.ParseConfig('sdl-config --libs')
    except OSError:
        pass
    SDL_ROOT.Append(LIBS = ['SDL_gfx','SDL_image','SDL_ttf'])

    if MAC: SDL_ROOT['FRAMEWORKS'] = ['OpenGL']
    else:   SDL_ROOT.Append(LIBS = ['libGL'])

if WINDOWS:
    SDL_ROOT.Append(LIBS=['opengl32.lib'])

    if not os.path.exists('SDL_deps'):
        SconsHelper.download_file_from_the_internets(
            'http://cloud.github.com/downloads/andyfischer/circa/SDL_deps.zip',
            'SDL_deps.zip')

        SconsHelper.unzip_file('SDL_deps.zip', '.')

    SDL_ROOT.Append(CPPPATH=['#SDL_deps/SDL-1.2.13/include'])
    SDL_ROOT.Append(CPPPATH=['#SDL_deps/SDL_image-1.2.7/include'])
    SDL_ROOT.Append(CPPPATH=['#SDL_deps/SDL_mixer-2.0.9/include'])
    SDL_ROOT.Append(CPPPATH=['#SDL_deps/SDL_ttf-2.0.9/include'])
    SDL_ROOT.Append(CPPPATH=['#plastic/deps/include'])
    SDL_ROOT.Append(LIBS=['SDL_deps/SDL-1.2.13/lib/SDL.lib'])
    SDL_ROOT.Append(LIBS=['SDL_deps/SDL-1.2.13/lib/SDLmain.lib'])
    SDL_ROOT.Append(LIBS=['SDL_deps/SDL_image-1.2.7/lib/SDL_image.lib'])
    SDL_ROOT.Append(LIBS=['SDL_deps/SDL_mixer-1.2.8/lib/SDL_mixer.lib'])
    SDL_ROOT.Append(LIBS=['SDL_deps/SDL_ttf-2.0.9/lib/SDL_ttf.lib'])
    SDL_ROOT.Append(LIBS=['plastic/deps/lib/glew32.lib'])

SDL_ROOT.Append(CPPPATH=['#src'])
SDL_ROOT.Append(LIBS = [circa_static_libs['release']])

Export('SDL_ROOT')

# Build Plastic project
SConscript('plastic/build.scons')
