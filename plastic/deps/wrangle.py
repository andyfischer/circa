#!/usr/bin/env python

"""

This script goes out on the internets, downloads the various dependencies needed
for Plastic, and reorganizes them into a build-friendly arrangement.

"""

import os, shutil

download_dir = './download_cache'

def download_file_from_the_internets(url, filename):
    destination_filename = os.path.join(download_dir, filename)
    print "Downloading "+url+" to "+destination_filename
    import urllib
    webFile = urllib.urlopen(url)
    localFile = open(filename, 'wb')
    localFile.write(webFile.read())
    webFile.close()
    localFile.close()
    return destination_filename

def mkdir(path):
    parent_dir = os.path.dirname(path)
    if parent_dir and not os.path.exists(parent_dir):
        mkdir(parent_dir)
    if not os.path.exists(path):
        os.mkdir(path)

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


mkdir(download_dir)

# glew
glew_zip = download_file_from_the_internets(
        "http://downloads.sourceforge.net/project/glew/glew/1.5.1/glew-1.5.1-win32.zip?use_mirror=softlayer",
        "glew-1.5.1.zip")
unzip_file(glew_zip, download_dir)
shutil.copytree(download_dir+'/glew/include', deploy_dir+'/include')
shutil.copytree(download_dir+'/glew/lib', deploy_dir+'/lib')
shutil.copytree(download_dir+'/glew/bin', deploy_dir+'/bin')


