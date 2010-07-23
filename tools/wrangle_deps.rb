
for windows:

        download_file_from_the_internets(
            'http://cloud.github.com/downloads/andyfischer/circa/SDL_deps.zip',
            'SDL_deps.zip')

        unzip_file('SDL_deps.zip', 'build/deps')

        remove SDL_deps.zip


# glew
glew_zip = download_file_from_the_internets(
        "http://downloads.sourceforge.net/project/glew/glew/1.5.1/glew-1.5.1-win32.zip?use_mirror=softlayer",
        "glew-1.5.1.zip")
unzip_file(glew_zip, download_dir)
shutil.copytree(download_dir+'/glew/include', 'include')
shutil.copytree(download_dir+'/glew/lib', 'lib')
shutil.copytree(download_dir+'/glew/bin', 'bin')


    

