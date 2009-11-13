#!/usr/bin/env python

import os, shutil

def download_file_from_the_internets(url, filename):
    print "Downloading "+url+" to "+filename
    import urllib
    webFile = urllib.urlopen(url)
    localFile = open(filename, 'wb')
    localFile.write(webFile.read())
    webFile.close()
    localFile.close()

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


download_dir = os.environ['CIRCA_HOME']+'/plastic/dependency_downloads'
deploy_dir = os.environ['CIRCA_HOME']+'/plastic/deps'
mkdir(download_dir)
mkdir(deploy_dir)

# glew
glew_zip = download_dir+"/glew-win32.zip"
download_file_from_the_internets("http://downloads.sourceforge.net/project/glew/glew/1.5.1/glew-1.5.1-win32.zip?use_mirror=softlayer", glew_zip)
unzip_file(glew_zip, download_dir)
shutil.copytree(download_dir+'/glew/include', deploy_dir+'/include')
shutil.copytree(download_dir+'/glew/lib', deploy_dir+'/lib')
shutil.copytree(download_dir+'/glew/bin', deploy_dir+'/bin')
