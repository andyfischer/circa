# 1.359, 1.105, 1.162, 1.101

import os, sys, ConfigParser, SCons

def fatal(msg):
    print "fatal:",msg
    exit(1)

# Load build.config
config = ConfigParser.ConfigParser()
config.read('build.config')

POSIX = os.name == 'posix'
WINDOWS = os.name == 'nt'

# Check for a Mac-like environment. On Macs, 'POSIX' is true, but there is
# a little bit of special behavior for using frameworks.
MAC = os.path.exists('/Library/Frameworks')

DEBUG = Environment(tools = ["default"], toolpath=".")
RELEASE = Environment(tools = ["default"], toolpath=".")
DEBUG['variant_name'] = 'debug'
RELEASE['variant_name'] = 'release'
ALL = [DEBUG, RELEASE]

if not os.path.exists('build'):
    os.mkdir('build')

# Common build flags
if POSIX:
    def common_flags(env):
        env.Append(CPPFLAGS=['-ggdb', '-Wall'])
        env.SetOption('num_jobs', 2)
    map(common_flags, ALL)
    DEBUG.Append(CPPDEFINES = ["DEBUG"])
    RELEASE.Append(CPPFLAGS=['-O1'])

if WINDOWS:
    def common_flags(env):
        env.Append(CPPDEFINES = ['WINDOWS'])
        env.Append(LINKFLAGS='/SUBSYSTEM:CONSOLE /MACHINE:X86 /DEBUG'.split())
    map(common_flags, ALL)
    DEBUG.Append(CPPFLAGS='/EHsc /W3 /MDd /Z7 /TP /Od'.split())
    DEBUG.Append(LINKFLAGS=['/NODEFAULTLIB:msvcrt.lib'])
    DEBUG.Append(CPPDEFINES = ["DEBUG", "_DEBUG"])
    RELEASE.Append(CPPFLAGS='/EHsc /W3 /MD /Z7 /TP /O2'.split())
    RELEASE.Append(CPPDEFINES = ["NDEBUG"])

def list_source_files(dir):
    for path in os.listdir(dir):
        full_path = os.path.join(dir,path)
        if not os.path.isfile(full_path): continue
        if not path.endswith('.cpp'): continue
        yield path

circa_static_libs = {}

# Define static library builds, save the results in circa_static_libs.
def circa_static_library(env):
    env = env.Clone()
    variant_name = env['variant_name']

    env.BuildDir('build/'+variant_name+'/src', 'src')
    env.Append(CPPPATH = ['src'])

    source_files = (list(list_source_files('src')) + 
            ['generated/'+filename for filename in list_source_files('src/generated')])
    source_files = filter(lambda f: f != 'main.cpp', source_files)

    circa_static_libs[variant_name] = env.StaticLibrary('build/'+variant_name+'/circa',
        ['build/'+variant_name+'/src/'+filename for filename in source_files])

map(circa_static_library, ALL)

# Define command-line app builds, save the results in circa_cl_builds
circa_cl_apps = {}

def circa_command_line_app(env):
    variant_name = env['variant_name']
    result = env.Program('build/bin/circa',
        'build/'+variant_name+'/src/main.cpp', LIBS=[circa_static_libs[variant_name]])
    circa_cl_apps[variant_name] = result
    return result
    
# Default build target is debug command-line binary.
Default(circa_command_line_app(DEBUG))

########################### SDL-based targets ###############################

def sdl_env(env):
    if POSIX:
        # import path so that we will find the correct sdl-config
        env['ENV']['PATH'] = os.environ['PATH']
        try:
            env.ParseConfig('sdl-config --cflags')
            env.ParseConfig('sdl-config --libs')
        except OSError:
            pass
        env.Append(LIBS = ['SDL_gfx','SDL_image','SDL_ttf'])

        if MAC: env['FRAMEWORKS'] = ['OpenGL']
        else:   env.Append(LIBS = ['libGL'])

    if WINDOWS:
        prebuild.sync_windows_sdl_deps()

        env.Append(LIBS=['opengl32.lib'])

        env.Append(CPPPATH=['#SDL_deps/SDL-1.2.13/include'])
        env.Append(CPPPATH=['#SDL_deps/SDL_image-1.2.7/include'])
        env.Append(CPPPATH=['#SDL_deps/SDL_mixer-2.0.9/include'])
        env.Append(CPPPATH=['#SDL_deps/SDL_ttf-2.0.9/include'])
        env.Append(CPPPATH=['#plastic/deps/include'])
        env.Append(LIBS=['SDL_deps/SDL-1.2.13/lib/SDL.lib'])
        env.Append(LIBS=['SDL_deps/SDL-1.2.13/lib/SDLmain.lib'])
        env.Append(LIBS=['SDL_deps/SDL_image-1.2.7/lib/SDL_image.lib'])
        env.Append(LIBS=['SDL_deps/SDL_mixer-1.2.8/lib/SDL_mixer.lib'])
        env.Append(LIBS=['SDL_deps/SDL_ttf-2.0.9/lib/SDL_ttf.lib'])
        env.Append(LIBS=['plastic/deps/lib/glew32.lib'])

    env.Append(CPPPATH=['#src'])

    variant_name = env['variant_name']

    # Append appropriate circa lib
    env.Append(LIBS = [circa_static_libs[variant_name]])

def build_plastic(env):
    env = env.Clone()
    env.BuildDir('build/plastic/src', 'plastic/src')
    sdl_env(env)

    source_files = list_source_files('plastic/src')

    env.Append(CPPDEFINES=['PLASTIC_USE_SDL'])

    result = env.Program('#build/bin/plas',
        source = ['build/plastic/src/'+f for f in source_files])

    # On Windows, embed manifest
    if WINDOWS:
        env.AddPostAction(result,
        'mt.exe -nologo -manifest plastic/windows/plastic.manifest.xml -outputresource:$TARGET;1')
    return result
    
plastic_variant = config.get('plastic', 'variant')
if plastic_variant == "release":
    plastic_env = RELEASE
elif plastic_variant == "debug":
    plastic_env = DEBUG
else:
    fatal("Unrecognzied variant under [plastic]: " + plastic_variant)
plastic = build_plastic(plastic_env)

Alias('plastic', plastic)
