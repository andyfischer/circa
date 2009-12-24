#!/usr/bin/env python

VERSION='0.2.0'
APPNAME='circa'
srcdir = '.'
blddir = 'build'

def init():
    pass

def set_options(opt):
    opt.tool_options('compiler_cxx')

def system_config(cmd):
    import subprocess
    p = subprocess.Popen(cmd.split(' '), stdout=subprocess.PIPE)
    return p.stdout.read()[:-1]

def configure(conf):
    conf.check_tool('compiler_cxx')

    import os
    mac = os.path.exists('/Library/Frameworks')

    if conf.env.CXX_NAME == 'gcc':
        conf.env.CXXFLAGS = ['-ggdb', '-Wall']
        conf.env.CXXDEFINES = 'DEBUG'
    elif conf.env.CXX_NAME == 'msvc':
        conf.env.CXXFLAGS = ['/EHsc', '/W3', '/MDd', '/Z7', '/TP', '/Od']
        conf.env.CXXDEFINES = ['DEBUG', '_DEBUG', 'WINDOWS']
    else:
        print "C++ compiler not recognized:", conf.env.CXX_NAME

    conf.env.CXXFLAGS += system_config('sdl-config --cflags').split(' ')
    conf.env.LINKFLAGS += system_config('sdl-config --libs').split(' ')
    conf.env.LIB = ['SDL_gfx', 'SDL_image', 'SDL_ttf']

    if mac:
        conf.env.FRAMEWORK = ['OpenGL']
    
def source_files(dir):
    import os
    def iter():
        for path in os.listdir(dir):
            full_path = os.path.join(dir,path)
            if not os.path.isfile(full_path): continue
            if not path.endswith('.cpp'): continue
            yield full_path
    return list(iter())

def build(bld):
    from tools import prebuild
    prebuild.main()

    circa_sources = (source_files('src') + source_files('src/generated'))

    # standalone command-line tool
    bld.new_task_gen(
        features = 'cxx cprogram',
        source = circa_sources,
        target = 'circa',
        includes = 'src')

    # plastic
    #plastic_sources = source_files('plastic/src')
    
    #bld.new_task_gen(
        #features = 'cxx cprogram',
        #source = plastic_sources,
        #target = 'plastic_app',
        #includes = 'src')


def shutdown():
    pass

