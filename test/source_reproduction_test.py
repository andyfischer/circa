#!/usr/bin/env python

import test_helper

def run_test(filename):
    return test_helper.compare_command_against_file('circa -s '+filename, filename)

if __name__ == "__main__":
    filename = sys.argv[1]
    result = run_test(filename)
    if result is not True:
        print result
    else:
        print "Suceess"
