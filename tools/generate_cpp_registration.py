# Copyright 2008 Andrew Fischer

import os, string

def list_cpp_files(directory):
    files = os.listdir(directory)
    names = []
    for file in files:
        if file.endswith('.cpp'):
            name = os.path.split(file)[1][:-4]
            names.append(name)
            #print "added "+name
    return (files, names)

def substitute(template, **subs):
    return string.Template(template).substitute(subs)

def write_text_file(filename, contents):
    output_file = open(filename, 'w')
    output_file.write(contents)
    output_file.write("\n")
    output_file.close()

def do_builtin_functions(directory, outputFile):

    (files, functionNames) = list_cpp_files(directory)

    def makeNamespace(functionName):
        return functionName.replace('-','_')+"_function"
    namespaces = map(makeNamespace, functionNames)

    def makePredeclaration(namespace):
        return "namespace "+namespace+" { void setup(Branch& kernel); }"

    def makeCall(namespace):
        return namespace+"::setup(kernel);"

    predeclarations = map(makePredeclaration, namespaces)
    calls = map(makeCall, namespaces)

    predeclarations.sort()
    calls.sort()

    output = substitute(BUILTIN_FUNCTIONS_TEMPLATE,
        predeclarations="\n".join(predeclarations),
        calls="\n    ".join(calls))

    write_text_file(outputFile, output)

def do_register_all_tests(directory, outputFile):
    (files, names) = list_cpp_files(directory)

    def makePredeclaration(name):
        return "namespace "+name+" { void register_tests(); }"

    def makeCall(name):
        return name+"::register_tests();"

    predeclarations = map(makePredeclaration, names)
    predeclarations.sort()
    calls = map(makeCall, names)
    calls.sort()

    output = substitute(REGISTER_ALL_TESTS_TEMPLATE,
            predeclarations = "\n".join(predeclarations),
            calls = "\n    ".join(calls))

    write_text_file(outputFile, output)

BUILTIN_FUNCTIONS_TEMPLATE = """
// Copyright 2008 Andrew Fischer

// This file is generated during the build process.
// You should probably not edit this file manually.

#include "common_headers.h"

#include "branch.h"

namespace circa {

$predeclarations

void setup_builtin_functions(Branch& kernel)
{
    $calls
}

} // namespace circa
"""

REGISTER_ALL_TESTS_TEMPLATE = """
// Copyright 2008 Andrew Fischer

// This file is generated during the build process.
// You should probably not edit this file manually.

#include "common_headers.h"

#include "testing.h"

namespace circa {

$predeclarations

void register_all_tests()
{
    gTestCases.clear();

    $calls
}

} // namespace circa
"""

if __name__ == "__main__":
    do_builtin_functions("../src/builtin_functions", "../src/setup_builtin_functions.cpp")
    do_builtin_functions("../src/tests", "../src/register_all_tests.cpp")
