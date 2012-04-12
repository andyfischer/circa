import os, sys, ConfigParser, SCons

def fatal(msg):
    print "fatal:",msg
    exit(1)

POSIX = os.name == 'posix'
WINDOWS = os.name == 'nt'

# Check for a Mac-like environment. On Macs, 'POSIX' is true, so it's mostly the same as
# a Unix build. But we look under /Library/Frameworks for some dependencies that are
# provided by Xcode.
OSX = os.path.exists('/Library/Frameworks')

TEST = Environment(tools = ["default"], toolpath=".",
        variant_name = 'test', variant_suffix='_t')
DEBUG = Environment(tools = ["default"], toolpath=".",
        variant_name = 'debug', variant_suffix='_d')
RELEASE = Environment(tools = ["default"], toolpath=".",
        variant_name = 'release', variant_suffix='')
all_envs = [DEBUG, RELEASE, TEST]

# Build flags
if POSIX:
    for env in all_envs:
        #env.Replace(CXX='clang++')
        env.Append(CPPFLAGS=['-ggdb', '-Wall'])
        env.Append(LINKFLAGS=['-ldl'])

        env.SetOption('num_jobs', 2)

    for env in [DEBUG,TEST]:
        env.Append(CPPDEFINES = ["DEBUG"])

    # TEST.Append(CPPFLAGS=['-fasm-blocks'])

    RELEASE.Append(CPPFLAGS=['-O1'])

if WINDOWS:
    for env in all_envs:
        env.Append(CPPDEFINES = ['WINDOWS'])
        env.Append(LINKFLAGS='/SUBSYSTEM:CONSOLE /DEBUG'.split())

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
    env.Append(CPPPATH = ['include'])

    generatedFiles = ['generated/'+filename for filename in list_source_files('src/generated')]

    source_files = (list_source_files('src')
        + ['tools/'+filename for filename in list_source_files('src/tools')]
        + ['generated/'+filename for filename in list_source_files('src/generated')])

    source_files.remove('main.cpp')
    source_files.remove('generated/all_source_files.cpp')

    if variant_name != 'test':
        source_files.remove('generated/all_tests.cpp')
        source_files.remove('generated/register_all_tests.cpp')

    baseName = 'circa' + env['variant_suffix']
    fullPath = 'build/'+baseName
    sources = ['build/'+variant_name+'/src/'+filename for filename in source_files]

    result = env.StaticLibrary(fullPath, sources)
    circa_libs[variant_name] = result[0]

# Define command-line app builds at build/circa_x
circa_cl_apps = {}

for env in all_envs:
    variant_name = env['variant_name']
    env.Append(CPPPATH = ['include'])
    binaryFile = 'build/circa' + env['variant_suffix']

    libs = [circa_libs[variant_name]]
    
    result = env.Program(binaryFile, 'build/'+variant_name+'/src/main.cpp',
            LIBS=libs)
    env.Append(LINKFLAGS = ["-all_load"])
    circa_cl_apps[variant_name] = result

    #if OSX and SHARED_LIBRARY:
    #    env.AddPostAction(result, 'install_name_tool -change build/libcirca_d.dylib '
    #            + '@executable_path/libcirca_d.dylib '+binaryFile)

# Default build target is release mode command-line app
Default(circa_cl_apps['release'])

# Unit tester app
unit_tester_env = DEBUG.Clone()
unit_tester_env.Append(CPPPATH=['src','tests/internal'])
unit_tester = unit_tester_env.Program('build/circa_test',
    ['tests/internal/' + s for s in list_source_files('tests/internal')],
    LIBS=circa_libs['debug'])
