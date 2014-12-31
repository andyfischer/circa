#!/usr/bin/env python3

import os

import NamesFile
import BuiltinFilesToC

def mkdir(dir):
    if os.path.exists(dir):
        return
    (parent, _) = os.path.split(dir)
    mkdir(parent)
    os.path.mkdir(dir)
    
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
    
def get_cpp_files_in_dir(dir):
    files = []
    for file in os.listdir(dir):
        if file.endswith('.cpp'):
            files.append(file)
    return files

def get_cpp_file_names(dir):
    return map(lambda s: s[:-4], get_cpp_files_in_dir(dir))

def register_all_tests():
    dir = 'src/tests'
    #print "cpp file names = " + str(get_cpp_file_names(dir))

    namespaces = get_cpp_file_names(dir)
    function_decls = '\n'.join(
            sorted(map(lambda n: 'namespace '+n+' { void register_tests(); }', namespaces)))
    function_calls = '\n    '.join(
            sorted(map(lambda n: n+'::register_tests();', namespaces)))

    return """\
// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

// This file is generated during the build process by ca-prebuild.py .
// You should probably not edit this file manually.

#include "common_headers.h"

#include "testing.h"

namespace circa {

%s

void register_all_tests()
{
    gTestCases.clear();

    %s
}

} // namespace circa""" % (function_decls, function_calls)
# end of register_all_tests


if 'CIRCA_HOME' in os.environ:
    os.chdir(os.environ['CIRCA_HOME'])

mkdir('src/generated')

# generate stdlib_script_text.cpp
write_text_file('src/generated/stdlib_script_text.cpp',
        BuiltinFilesToC.builtin_files_to_c('src/ca', '$builtins/'))

# generate all_tests.cpp
def source_files(dir):
    for path in os.listdir(dir):
        if not os.path.isfile(os.path.join(dir,path)): continue
        if not path.endswith('.cpp'): continue
        yield path
def test_cpps():
    for file in get_cpp_files_in_dir('src/tests'):
        yield "tests/"+file
def library_sources():
    for file in get_cpp_files_in_dir('src'):
        if file == 'main.cpp': continue
        yield file
def include_list(items):
    generated_cpp = []
    for item in items:
        generated_cpp.append('#include "'+item+'"')
    return "\n".join(generated_cpp)

#write_text_file('src/generated/all_tests.cpp',
#    include_list(['../'+file for file in test_cpps()]))

def all_source_files():
    for file in get_cpp_files_in_dir('src'):
        if file == 'main.cpp':
            continue
        yield "../" + file
    for file in get_cpp_files_in_dir('src/command_line'):
        yield "../command_line/" + file
    yield "./stdlib_script_text.cpp"

write_text_file('src/generated/all_source_files.cpp',
    include_list(all_source_files()))

NamesFile.regenerate_file()
