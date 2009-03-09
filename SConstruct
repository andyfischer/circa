
import os, sys, glob 

# generate setup_builtin_functions.cpp and register_all_tests.cpp
from scripts import generate_cpp_registration
generate_cpp_registration.do_builtin_functions(
    'src/builtin_functions',
    'src/setup_builtin_functions.cpp')
generate_cpp_registration.do_register_all_tests(
    'src/tests',
    'src/register_all_tests.cpp')

ENV = Environment(tools = ["default"], toolpath=".")

ENV.Append(CPPFLAGS=['-ggdb'])
ENV.Append(CPPFLAGS=['-Wall'])

ENV.Append(CPPDEFINES = ["_DEBUG"])
ENV.Append(CPPDEFINES = ["DEBUG"])

ENV.BuildDir('build/src', 'src',duplicate=0)
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

source_directory('src', excludes=['test_program.cpp', 'main.cpp'])
source_directory_into_one_cpp('src/tests', 'all_tests')
source_directory_into_one_cpp('src/builtin_functions', 'all_builtin_functions')

circa_slib = ENV.StaticLibrary('lib/circa', BUILD_FILES)
circa_so = ENV.SharedLibrary('lib/circa', BUILD_FILES)

circaBinary = ENV.Program('bin/circa', 'build/src/main.cpp', LIBS=[circa_slib])

ENV.SetOption('num_jobs', 2)
ENV.Default(circaBinary)

############################## cgame ###############################

CASDL_ENV = Environment()

# import path so that we will find the correct sdl-config
CASDL_ENV['ENV']['PATH'] = os.environ['PATH']

CASDL_ENV.ParseConfig('sdl-config --cflags')
CASDL_ENV.ParseConfig('sdl-config --libs')

CASDL_ENV.Append(LIBS = ['SDL_gfx'])
CASDL_ENV.Append(CPPFLAGS=['-ggdb'])
CASDL_ENV.Append(CPPPATH=['src'])
CASDL_ENV.Append(LIBPATH = "lib")
CASDL_ENV.Append(LIBS = ['circa'])

CASDL_ENV.Program('bin/cgame', 'cgame/main.cpp')

CASDL_ENV.Alias('cgame', 'bin/cgame')
