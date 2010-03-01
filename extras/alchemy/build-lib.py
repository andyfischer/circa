#!/usr/bin/env python

import os
os.chdir(os.environ['CIRCA_HOME'])

from tools import prebuild

out = []

def every_source_file():
    import glob
    for f in glob.glob('src/*.cpp'):
        yield f
    for f in glob.glob('src/generated/*.cpp'):
        yield f

command = ('g++ -Isrc -DOSX -O3 -Wall -swc '
        + " ".join(every_source_file())
        + ' -o extras/alchemy/libcirca.swc')

print command
os.system(command)
