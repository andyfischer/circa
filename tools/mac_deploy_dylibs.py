#!/usr/bin/env python

import os,sys,shutil,time
time.sleep(1)

files = {
    '/opt/local/lib/libSDL-1.2.0.dylib': 'bin/libSDL-1.2.0.dylib',
    '/opt/local/lib/libSDL_gfx.0.dylib': 'bin/libSDL_gfx.0.dylib',
    '/opt/local/lib/libSDL_image-1.2.0.dylib': 'bin/libSDL_image-1.2.0.dylib',
    '/opt/local/lib/libSDL_ttf-2.0.0.dylib': 'bin/libSDL_ttf-2.0.0.dylib',
    '/opt/local/lib/libfreetype.6.dylib': 'bin/libfreetype.6.dylib',
    '/opt/local/lib/libz.1.dylib': 'bin/libz.1.dylib'
}


def update_dylib(path):
  for (old,new) in files.items():
    os.system('install_name_tool -change %s %s %s' % (old, new, path))

for (old,new) in files.items():
  shutil.copy(old,new)

for (old,new) in files.items():
  update_dylib(new)

update_dylib('plastic')
