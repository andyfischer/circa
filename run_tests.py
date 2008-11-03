#!/usr/bin/env python

import os,sys

TESTS_RUN = 0
FAILURES = []

def do_output_test(*args):
    from tests import output_test
    global TESTS_RUN
    global FAILURES
    
    result = output_test.run_test(*args)
    if result is not True:
        FAILURES.append(result)

    TESTS_RUN += 1

def print_results():
    output = "Ran %d tests." % TESTS_RUN

    if not FAILURES:
        output += " All tests passed."
        print output
    else:
        output += " %d tests failed:" % len(FAILURES)
        print output

        for failure in FAILURES:
            print failure

do_output_test("tests/boolean.ca")

print_results()
