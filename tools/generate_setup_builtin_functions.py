# Copyright 2008 Andrew Fischer

import os, string

def run(functionsDirectory, outputFilename):

    print "dir is: " +functionsDirectory

    files = os.listdir(functionsDirectory)
    functionNames = []
    for file in files:
        if file.endswith('.cpp'):
            function_name = os.path.split(file)[1][:-4]
            functionNames.append(function_name)
            #print "added "+function_name

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

    output = string.Template(TEMPLATE).substitute({
        'predeclarations':"\n".join(predeclarations),
        'calls':"\n    ".join(calls)})

    output_file = open(outputFilename, 'w')
    output_file.write(output)
    output_file.write("\n")
    output_file.close()


TEMPLATE = """
// Copyright 2008 Andrew Fischer

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

if __name__ == "__main__":
    run("../src/builtin_functions", "../src/setup_builtin_functions.cpp")
