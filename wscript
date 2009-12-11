#!/usr/bin/env python

VERSION='0.2.0'
APPNAME='circa'
srcdir = '.'
blddir = 'build'

def init():
    pass

def set_options(opt):
    opt.tool_options('compiler_cxx')

def configure(conf):
    conf.check_tool('compiler_cxx')

    if conf.env.CXX_NAME == 'gcc':
        conf.env.CXXFLAGS = ['-ggdb', '-Wall']
        conf.env.CXXDEFINES = 'DEBUG'
    elif conf.env.CXX_NAME == 'msvc':
        conf.env.CXXFLAGS = ['/EHsc', '/W3', '/MDd', '/Z7', '/TP', '/Od']
        conf.env.CXXDEFINES = ['DEBUG', '_DEBUG', 'WINDOWS']
    else:
        print "C++ compiler not recognized:", conf.env.CXX_NAME

def source_files(dir):
    import os
    def iter():
        for path in os.listdir(dir):
            full_path = os.path.join(dir,path)
            if not os.path.isfile(full_path): continue
            if not path.endswith('.cpp'): continue
            yield path
    return list(iter())

def build(bld):
    circa_sources = (source_files('src') +
        ['generated/'+file for file in source_files('src/generated')])

    circa_sources = ['src/'+file for file in circa_sources]

    # standalone command-line tool
    bld.new_task_gen(
        features = 'cxx cprogram',
        source = circa_sources,
        target = 'circa',
        includes = 'src')

def shutdown():
    pass

