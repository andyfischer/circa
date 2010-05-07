#!/usr/bin/env python

"""

This script goes out on the internets, downloads the various dependencies needed
for Plastic, and reorganizes them into a build-friendly arrangement.

"""

import os, shutil

def mkdir(path):
    parent_dir = os.path.dirname(path)
    if parent_dir and not os.path.exists(parent_dir):
        mkdir(parent_dir)
    if not os.path.exists(path):
        os.mkdir(path)

def download_file_from_the_internets(url, filename):
    print "Downloading "+url+" to "+filename
    import urllib
    webFile = urllib.urlopen(url)
    localFile = open(filename, 'wb')
    localFile.write(webFile.read())
    webFile.close()
    localFile.close()
    return filename

def unzip_file(filename, dir):
    print "Unzipping "+filename+" to "+dir
    import zipfile

    if not os.path.exists(dir): os.mkdir(dir)

    zf = zipfile.ZipFile(filename)
    for name in zf.namelist():
        path = os.path.join(dir, name)
        if name.endswith('/'):
            mkdir(path)
        else:
            f = open(path, 'wb')
            f.write(zf.read(name))
            f.close()

def rmtree(path):
    if os.path.exists(path):
        shutil.rmtree(path)

def copytree(src, dst):
    # This is similar to shutil.copytree, but it doesn't error if the destination directory
    # already exists.
    mkdir(dst)
    for name in os.listdir(src):
        srcname = os.path.join(src, name)
        dstname = os.path.join(dst, name)
        if os.path.isdir(srcname):
            copytree(srcname, dstname)
        else:
            shutil.copy2(srcname, dstname)


print 'Cleaning build/temp and build/deps'
rmtree('build/deps')
rmtree('build/temp')

mkdir('build/temp')
mkdir('build/deps/include')
mkdir('build/deps/lib')

# glew
glew_zip = download_file_from_the_internets(
        "http://downloads.sourceforge.net/project/glew/glew/1.5.1/glew-1.5.1-win32.zip?use_mirror=softlayer",
        "build/temp/glew-1.5.1.zip")
unzip_file(glew_zip, 'build/temp')
copytree('build/temp/glew/include', 'build/deps/include')
copytree('build/temp/glew/lib', 'build/deps/lib')
copytree('build/temp/glew/bin', 'build/bin')

# SDL
SDL_zip = download_file_from_the_internets(
        "http://cloud.github.com/downloads/andyfischer/circa/SDL_deps.zip",
        "build/temp/SDL_deps.zip")
unzip_file(SDL_zip, 'build/temp')

copytree('build/temp/SDL_deps/SDL-1.2.13/include',      'build/deps/include')
copytree('build/temp/SDL_deps/SDL-1.2.13/lib',          'build/deps/lib')
copytree('build/temp/SDL_deps/SDL_image-1.2.7/include', 'build/deps/include')
copytree('build/temp/SDL_deps/SDL_image-1.2.7/lib',     'build/deps/lib')
copytree('build/temp/SDL_deps/SDL_mixer-1.2.8/include', 'build/deps/include')
copytree('build/temp/SDL_deps/SDL_mixer-1.2.8/lib',     'build/deps/lib')
copytree('build/temp/SDL_deps/SDL_ttf-2.0.9/include',   'build/deps/include')
copytree('build/temp/SDL_deps/SDL_ttf-2.0.9/lib',       'build/deps/lib')
copytree('build/temp/SDL_deps/bin',                     'build/bin')

print 'Cleaning build/temp'
rmtree('build/temp')

print 'Include files are in build/deps/include'
print 'Library files are in build/deps/lib'
print 'Binary files are in build/bin'
