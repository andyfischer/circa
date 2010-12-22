#!/usr/bin/env python

import glob, os, sys
import test_helper

def to_test_case(filename):
    def func():
        try:
            return test_helper.compare_command_against_file('circa -s '+filename, filename)
        except Exception,e:
            return "On file "+filename+": "+str(e)
    return test_helper.TestCase("source repro: "+filename, func)

suite = []

for filename in glob.glob(os.environ['CIRCA_HOME'] + "/tests/samples/*.ca"):
    suite.append(to_test_case(filename))
for filename in glob.glob(os.environ['CIRCA_HOME'] + "/plastic/demos/*.ca"):
    suite.append(to_test_case(filename))

if __name__=='__main__':
    test_helper.run_tests_and_print_results(suite)
