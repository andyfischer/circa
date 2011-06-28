#!/usr/bin/env python

"""
This script goes out on the internets, downloads the various dependencies needed
for Plastic, and reorganizes them into a build-friendly arrangement.
"""

import os, shutil

TempDir = 'build/temp'

def mkdir(path):
    parent_dir = os.path.dirname(path)
    if parent_dir and not os.path.exists(parent_dir):
        mkdir(parent_dir)
    if not os.path.exists(path):
        os.mkdir(path)

def download_file_from_the_internets(url, localFilename=None):
    if localFilename is None:
        localFilename = os.path.join(TempDir, os.path.basename(url))
    import urllib
    webFile = urllib.urlopen(url)
    localFile = open(localFilename, 'wb')
    localFile.write(webFile.read())
    webFile.close()
    localFile.close()
    return localFilename

def unzip_file(filename, dir):
    import zipfile

    mkdir(dir)

    zf = zipfile.ZipFile(filename)
    for name in zf.namelist():
        path = os.path.join(dir, name)
        if name.endswith('/'):
            mkdir(path)
        else:
            f = open(path, 'wb')
            f.write(zf.read(name))
            f.close()

def download_and_unzip(url):
    file = download_file_from_the_internets(url)
    unzip_file(file, TempDir)

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


def main():
    #print 'Cleaning build/temp'
    #rmtree('build/temp')

    mkdir('deps')
    mkdir('build/temp')

    # glew
    download_and_unzip(
            "http://downloads.sourceforge.net/project/glew/glew/1.5.1/glew-1.5.1-win32.zip")
    copytree('build/temp/glew/include', 'deps/include')
    copytree('build/temp/glew/lib', 'deps/lib')
    copytree('build/temp/glew/bin', 'deps/bin')

    # SDL
    download_and_unzip("http://www.libsdl.org/release/SDL-devel-1.2.14-VC8.zip")
    copytree('build/temp/SDL-1.2.14/include', 'deps/include')
    copytree('build/temp/SDL-1.2.14/lib', 'deps/lib')

    # SDL_ttf
    download_and_unzip("http://www.libsdl.org/projects/SDL_ttf/release/SDL_ttf-devel-2.0.10-VC.zip")
    copytree('build/temp/SDL_image-1.2.10/include', 'deps/include')
    copytree('build/temp/SDL_image-1.2.10/lib', 'deps/lib')

    # SDL_image
    download_and_unzip("http://www.libsdl.org/projects/SDL_image/release/SDL_image-devel-1.2.10-VC.zip")
    copytree('build/temp/SDL_ttf-2.0.10/include', 'deps/include')
    copytree('build/temp/SDL_ttf-2.0.10/lib', 'deps/lib')

    # Box2d
    download_and_unzip("http://box2d.googlecode.com/files/Box2D_v2.1.2.zip")
    copytree('build/temp/Box2D_v2.1.2/Box2D/Box2D', 'deps/include/Box2D')


if __name__ == '__main__':
    main()
