
import os, shutil, zipfile

AppName = 'ImprovAlpha4.app'

def mkdir(path):
    if os.path.exists(path):
        return
    os.mkdir(path)

def rmdir(path):
    if not os.path.exists(path):
        return
    if os.path.isdir(path):
        shutil.rmtree(path)
    else:
        os.remove(path)

def copy(path, dest):

    if os.path.isdir(path):
        # Copy the whole folder, not just its contents
        folderName = os.path.split(path)[1]
        dest = os.path.join(dest, folderName)
        rmdir(dest)
        shutil.copytree(path, dest)
    else:
        mkdir(dest)
        os.copy2(path,dest)



macZip = zipfile.ZipFile('ImprovAlpha4.zip', 'w')

def add(dir):
    for root, dirnames, filenames in os.walk(dir):
        for file in filenames:
            fullpath = os.path.join(root,file)
            zipname = fullpath
            zipname = zipname.replace('improv.app','ImprovAlpha4.app')
            print 'writing ',fullpath,' as ',zipname
            macZip.write(fullpath, zipname, zipfile.ZIP_DEFLATED)

add('improv.app')
add('ca')
add('demos')
