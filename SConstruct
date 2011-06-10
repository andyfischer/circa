"""
Primary build file for Circa.
See docs/build_targets.md for more information on the available targets and variants.
"""

import os, sys, ConfigParser, SCons

def fatal(msg):
    print "fatal:",msg
    exit(1)

# Load build.config
config = ConfigParser.ConfigParser()
config.read('build.config')

POSIX = os.name == 'posix'
WINDOWS = os.name == 'nt'

# Check for a Mac-like environment. On Macs, 'POSIX' is true, so it's mostly the same as
# a Unix build. But we look under /Library/Frameworks for some dependencies that are
# provided by Xcode.
OSX = os.path.exists('/Library/Frameworks')

DEBUG = Environment(tools = ["default"], toolpath=".",
        variant_name = 'debug', variant_suffix='_d')
RELEASE = Environment(tools = ["default"], toolpath=".",
        variant_name = 'release', variant_suffix='_r')
TEST = Environment(tools = ["default"], toolpath=".",
        variant_name = 'test', variant_suffix='_t')
all_envs = [DEBUG, RELEASE, TEST]

SHARED_LIBRARY = config.get('circa', 'shared_library') == 'true'

# Build flags
if POSIX:
    for env in all_envs:
        env.Append(CPPFLAGS=['-ggdb', '-Wall'])
        env.Append(LINKFLAGS=['-ldl'])

        if config.get('circa', 'gprof_support') == 'true':
            env.Append(CPPFLAGS=['-pg'])
            env.Append(LINKFLAGS=['-pg'])

        env.SetOption('num_jobs', 2)

    for env in [DEBUG,TEST]:
        env.Append(CPPDEFINES = ["DEBUG"])

    RELEASE.Append(CPPFLAGS=['-O1'])

if WINDOWS:
    for env in all_envs:
        env.Append(CPPDEFINES = ['WINDOWS'])
        env.Append(LINKFLAGS='/SUBSYSTEM:CONSOLE /MACHINE:X86 /DEBUG'.split())

    for env in [DEBUG,TEST]:
        env.Append(CPPFLAGS='/EHsc /W3 /MDd /Z7 /TP /Od'.split())
        env.Append(LINKFLAGS=['/NODEFAULTLIB:msvcrt.lib'])
        env.Append(CPPDEFINES = ["DEBUG", "_DEBUG"])
    RELEASE.Append(CPPFLAGS='/EHsc /W3 /MD /Z7 /TP /O2'.split())
    RELEASE.Append(CPPDEFINES = ["NDEBUG"])

TEST.Append(CPPDEFINES = ['CIRCA_TEST_BUILD'])

def list_source_files(dir):
    result = []
    for path in os.listdir(dir):
        full_path = os.path.join(dir,path)
        if not os.path.isfile(full_path): continue
        if not path.endswith('.cpp'): continue
        result.append(path)
    return result

# Define library builds, save the results in circa_libs.
circa_libs = {}

for env in all_envs:
    env = env.Clone()
    variant_name = env['variant_name']

    env.VariantDir('build/'+variant_name+'/src', 'src')
    env.Append(CPPPATH = ['src'])

    generatedFiles = ['generated/'+filename for filename in list_source_files('src/generated')]


    source_files = (list_source_files('src')
        + ['generated/'+filename for filename in list_source_files('src/generated')])

    source_files.remove('main.cpp')

    if variant_name != 'test':
        source_files.remove('generated/all_tests.cpp')

    baseName = 'circa' + env['variant_suffix']
    fullPath = 'build/'+baseName
    sources = ['build/'+variant_name+'/src/'+filename for filename in source_files]

    buildStep = env.SharedLibrary if SHARED_LIBRARY else env.StaticLibrary

    result = buildStep(fullPath, sources)
    circa_libs[variant_name] = result

    if OSX and SHARED_LIBRARY:
        actualFile = 'lib'+baseName+'.dylib'
        env.AddPostAction(result, 'install_name_tool -id @executable_path/'+actualFile
                + ' build/'+actualFile)

# Define command-line app builds at build/circa_x
circa_cl_apps = {}

for env in all_envs:
    variant_name = env['variant_name']
    env.Append(CPPPATH = ['src'])
    binaryFile = 'build/circa' + env['variant_suffix']

    libs = [circa_libs[variant_name]]
    
    result = env.Program(binaryFile, 'build/'+variant_name+'/src/main.cpp',
            LIBS=libs)
    circa_cl_apps[variant_name] = result

    if OSX and SHARED_LIBRARY:
        env.AddPostAction(result, 'install_name_tool -change build/libcirca_d.dylib '
                + '@executable_path/libcirca_d.dylib '+binaryFile)
    
# Default build target is debug command-line binary.
circa_cl = circa_cl_apps['debug']
Default(circa_cl)

########################### Plastic ###############################

def use_sdl(env):
    if POSIX:
        # import path so that we will find the correct sdl-config
        env['ENV']['PATH'] = os.environ['PATH']
        try:
            env.ParseConfig('sdl-config --cflags')
            env.ParseConfig('sdl-config --libs')
        except OSError:
            pass
        env.Append(LIBS = ['SDL_gfx','SDL_image','SDL_ttf'])

        if OSX:
            env['FRAMEWORKS'] = ['OpenGL']
            env.Append(CPPDEFINES = ['PLASTIC_OSX'])
        else:
            env.Append(LIBS = ['libGL'])

    if WINDOWS:
        env.Append(LIBS=['opengl32.lib'])

        env.Append(CPPPATH=['build/deps/include'])
        env.Append(LIBS=['build/deps/lib/SDL.lib'])
        env.Append(LIBS=['build/deps/lib/SDLmain.lib'])
        env.Append(LIBS=['build/deps/lib/SDL_image.lib'])
        env.Append(LIBS=['build/deps/lib/SDL_mixer.lib'])
        env.Append(LIBS=['build/deps/lib/SDL_ttf.lib'])
        env.Append(LIBS=['build/deps/lib/glew32.lib'])

    env.Append(CPPPATH=['#src'])

    variant_name = env['variant_name']

    # Append appropriate circa lib
    env.Append(LIBS = [circa_libs[variant_name]])

def use_fmod(env):
    base = 'build/deps/fmod/api/'
    env.Append(CPPPATH=[base + 'inc'])
    #env.Append(LINKFLAGS=['-arch','i386','-arch','ppc','-arch','x86_64'])
    #env.Append(LINKFLAGS=['-l'+ base + 'lib/libfmodex.dylib'])
    env.Append(LIBPATH=[base + 'lib'])
    env.Append(LIBS=['fmodex'])

def use_box2d(env):
    env.Append(LIBS=['box2d'])

def use_liblo(env):
    env.Append(LIBS=['lo'])

# Define plastic targets at build/plas_x
for env in all_envs:
    env = env.Clone()
    variantName = env['variant_name']
    env.VariantDir('build/'+variantName+'/plastic/src', 'plastic/src')
    use_sdl(env)
    #use_fmod(env)
    use_box2d(env)
    use_liblo(env)

    source_files = list_source_files('plastic/src')

    env.Append(CPPDEFINES=['PLASTIC_USE_SDL'])

    binaryName = 'plas'+env['variant_suffix']

    result = env.Program('#build/' + binaryName,
        source = ['build/'+variantName+'/plastic/src/'+f for f in source_files])

    # On Windows, embed manifest
    if WINDOWS:
        env.AddPostAction(result,
        'mt.exe -nologo -manifest plastic/windows/plastic.manifest -outputresource:$TARGET;1')

    if OSX:
        env.AddPostAction(result,
            'install_name_tool -change ./libfmodex.dylib build/deps/fmod/api/lib/libfmodex.dylib $TARGET')
