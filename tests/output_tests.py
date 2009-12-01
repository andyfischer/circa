#!/usr/bin/env python

import glob, os, sys
import test_helper

def get_expected_output_filename(script_filename):
    return "tests/expected_output/"+os.path.split(script_filename)[1]+'.expected_output'

def test_file(filename):
    """
    Performs an output test on the given filename. This file should be a circa
    source code file, which we will execute. There should also be a file
    called filename.expected_output which contains what we expect the script
    to print to stdout.

    Returns: True if we succeeded, a string error message if we failed.
    """
    return test_helper.compare_command_against_file('circa '+filename,
        get_expected_output_filename(filename))

def to_test_case(filename):
    def func():
        return test_file(filename)
    return test_helper.TestCase("output test: "+filename, func)

suite = []
for filename in glob.glob(os.environ['CIRCA_HOME'] + "/tests/samples/*.ca"):
    suite.append(to_test_case(filename))

def accept_output(filename):
    if not os.path.exists(filename):
        print "File not found: " +filename
        return False

    actual_output = test_helper.run_command("circa "+filename)

    f = open(get_expected_output_filename(filename), 'w')
    f.write(actual_output)

    print "Accepted output for "+filename+":"
    print actual_output

if __name__ == "__main__":
    if len(sys.argv) == 1:
        test_helper.run_tests_and_print_results(suite)
        exit(0)

    if sys.argv[1] == "accept":
        filename = sys.argv[2]
        accept_output(filename)
        exit(0)

    tests = [to_test_case(sys.argv[1])]
    test_helper.run_tests_and_print_results(tests)


