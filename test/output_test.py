#!/usr/bin/env python

import os, sys
import test_helper

def run_test(filename):
    """
    Performs an output test on the given filename. This file should be a circa
    source code file, which we will execute. There should also be a file
    called filename.expected_output which contains what we expect the script
    to print to stdout.

    Returns: True if we succeeded, a string error message if we failed.
    """

    if not os.path.exists(filename):
        return "Couldn't find file: " + filename

    expectedOutputFile = filename + ".expected_output"

    if not os.path.exists(expectedOutputFile):
        return "Couldn't find file: " + expectedOutputFile

    expectedOutput = test_helper.loadFile(expectedOutputFile)

    command = "circa " + filename

    (stdin, stdout, stderr) = os.popen3(command)

    numLines = 0
    for line in expectedOutput.split('\n'):
        # Make sure there is nothing in stderr
        stderr_line = stderr.readline()
        if stderr_line != "":
            stderr_line = stderr_line[:-1] # remove trailing newline
            return "While running %s, received error: %s" % (command, stderr_line)

        # Read a line from stdout and check it against expected
        stdout_line = stdout.readline()
        stdout_line = stdout_line[:-1] # remove trailing newline

        if line != stdout_line:
            return ("Unexpected output for %s on line %d:\nExpected: %s\nObserved: %s"
                    % (filename, numLines, line, stdout_line))

        numLines += 1

    return True

if __name__ == "__main__":
    filename = sys.argv[1]
    result = run_test(filename)
    if result is not True:
        print result
    else:
        print "Suceess"
