#!/usr/bin/env python

VERSION='0.0.1'
APPNAME='circa'
srcdir = 'src'
blddir = 'build'

def init():
    pass

def set_options(opt):
    opt.tool_options('compiler_cxx')

def configure(conf):
    conf.check_tool('compiler_cxx')
    conf.env.CXXFLAGS = ['-Wall', '-ggdb']

    # testcase for variants, look below
    #dbg = conf.env.copy()
    #rel = conf.env.copy()

    #rel.set_variant('release')
    #conf.set_env_name('release', rel)
    #conf.setenv('release')
    #conf.env.CXXFLAGS = ['-O2']

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

    # todo: figure out how to properly specify src directory
    circa_sources = ['src/'+file for file in circa_sources]

    bld.new_task_gen(
        features = 'cxx cprogram',
        source = circa_sources,
        target = 'circa_app',
        defines = '',
        includes = 'src',
        destfile = 'build/circa')

def shutdown():
    pass

