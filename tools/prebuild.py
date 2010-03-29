#!/usr/bin/env python

import os

def read_text_file(path):
    if not os.path.exists(path):
        return ""
    f = open(path, 'r')
    return f.read()[:-1]

def write_text_file(path, contents):
    # read the file first, and only write if the contents are different. This saves
    # us from triggering a rebuild on build systems that check the modified time.
    if contents == read_text_file(path):
        return

    f = open(path, 'w')
    f.write(contents)
    f.write("\n")
    
def download_file_from_the_internets(url, filename):
    print "Downloading "+url
    import urllib
    webFile = urllib.urlopen(url)
    localFile = open(filename, 'wb')
    localFile.write(webFile.read())
    webFile.close()
    localFile.close()

def unzip_file(filename, dir):
    print "Unzipping "+filename+" to "+dir
    import os, zipfile

    if not os.path.exists(dir): os.mkdir(dir)

    zf = zipfile.ZipFile(filename)
    for name in zf.namelist():
        path = os.path.join(dir, name)
        if name.endswith('/'):
            if not os.path.exists(path): os.mkdir(path)
        else:
            f = open(path, 'wb')
            f.write(zf.read(name))
            f.close()

def sync_windows_sdl_deps():
    if not os.path.exists('SDL_deps'):
        download_file_from_the_internets(
            'http://cloud.github.com/downloads/andyfischer/circa/SDL_deps.zip',
            'SDL_deps.zip')

        unzip_file('SDL_deps.zip', '.')

def main():

    if not os.path.exists('src/generated'):
        os.mkdir('src/generated')

    # generate setup_builtin_functions.cpp and register_all_tests.cpp
    import generate_cpp_registration
    write_text_file('src/generated/setup_builtin_functions.cpp',
            generate_cpp_registration.do_builtin_functions('src/builtin_functions'))
    write_text_file('src/generated/register_all_tests.cpp',
            generate_cpp_registration.do_register_all_tests('src/tests'))

    # generate builtin_script_text.cpp
    import text_file_to_c
    write_text_file('src/generated/builtin_script_text.cpp',
            text_file_to_c.generate("src/ca/builtins.ca", "BUILTIN_SCRIPT_TEXT"))

    def source_files(dir):
        for path in os.listdir(dir):
            if not os.path.isfile(os.path.join(dir,path)): continue
            if not path.endswith('.cpp'): continue
            yield path

    def builtin_function_cpps():
        for file in source_files('src/builtin_functions'):
            yield "builtin_functions/"+file
    def test_cpps():
        for file in source_files('src/tests'):
            yield "tests/"+file
    def library_sources():
        for file in source_files('src'):
            if file == 'main.cpp': continue
            yield file

    def include_list(items):
        generated_cpp = []
        for item in items:
            generated_cpp.append('#include "'+item+'"')
        return "\n".join(generated_cpp)

    write_text_file('src/generated/all_tests.cpp', include_list(test_cpps()))
    write_text_file('src/generated/all_builtin_functions.cpp',
            include_list(builtin_function_cpps()))
    write_text_file('src/generated/all_builtin_types.cpp',
        include_list(['builtin_types/'+file for file in source_files('src/builtin_types')]))
            

if __name__ == '__main__':
    main()
