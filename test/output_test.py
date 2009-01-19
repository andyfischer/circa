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

    return test_helper.compare_command_against_file('circa '+filename, filename+'.expected_output')

if __name__ == "__main__":
    filename = sys.argv[1]
    result = run_test(filename)
    if result is not True:
        print result
    else:
        print "Suceess"
