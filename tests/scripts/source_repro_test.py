#!/usr/bin/env python

import glob,os,sys
import test_helper


def test_file(filename):
    try:
        return test_helper.compare_command_against_file('circa -s '+filename, filename)
    except Exception,e:
        return "On file "+filename+": "+str(e)

def get_all_the_files_to_test():
    for filename in glob.glob(os.environ['CIRCA_HOME'] + "/tests/samples/*.ca"):
        yield filename
    for filename in glob.glob(os.environ['CIRCA_HOME'] + "/plastic/demos/*.ca"):
        yield filename

def test_all():
    tests_run = 0
    failures = []
    for filename in get_all_the_files_to_test():
        tests_run += 1
        result = test_file(filename)
        if result is not True:
            failures.append(result)

    summary = "Ran "+str(tests_run)+" tests."
    if not failures:
        print summary + " All tests passed."
    else:
        print summary + " " + str(len(failures)) + " failure(s):"
        
        for failure in failures:
            print failure

if __name__ == "__main__":

    if len(sys.argv) > 1:
        result = run_test(sys.argv[1])
        if result is not True:
            print result
        else:
            print "Success"
    else:
        test_all()
