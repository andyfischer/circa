#!/usr/bin/env python

import os, shutil, zipfile
from glob import glob

def mkdir(path):
    if not os.path.exists(path):
        os.mkdir(path)

def recr_mkdir(path):
    if not os.path.exists(path):
        parent = os.path.split(path)[0]
        if parent:
            recr_mkdir(parent)
        os.mkdir(path)

def rm(path):
    if os.path.isdir(path): shutil.rmtree(path)
    else: os.remove(path)

def rm_glob(pattern):
    for file in glob(pattern):
        rm(file)

def copy_glob(pattern, dest):
    for file in glob(pattern):
        shutil.copy(file, dest)

os.chdir(os.environ['CIRCA_HOME'])

version_name='plastic_win32_cl_alpha1'

target_dir='build/deploy/'+version_name
recr_mkdir(target_dir)
rm_glob(target_dir+'/*')

shutil.copy('build/bin/plas.exe', target_dir+'/plastic.exe')
shutil.copytree('plastic/assets', target_dir+'/assets')
shutil.copy('plastic/runtime.ca', target_dir)
shutil.copytree('plastic/demos', target_dir+'/demos')

# remove some demos that aren't ready yet
rm(target_dir+'/demos/meshtest.ca')
rm(target_dir+'/demos/ramps.ca')
rm(target_dir+'/demos/tree.ca')

#mkdir(target_dir+'/bin')
copy_glob('SDL_deps/bin/*', target_dir)
copy_glob('c:/WINDOWS/WinSxS/x86_Microsoft.VC80.CRT_1fc8b3b9a1e18e3b_8.0.50727.762_x-ww_6b128700/*.dll', target_dir)

zfile = zipfile.ZipFile(target_dir + '.zip', 'w')

def walk(dir):
    for (dirpath,dirnames,filenames) in os.walk(dir):
        for filename in filenames:
            yield os.path.join(dirpath,filename)

for file in walk(target_dir):
    file_in_archive = file[len('build/deploy/'):]
    zfile.write(file, file_in_archive)
