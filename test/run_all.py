#!/usr/bin/env python

import os,sys

TESTS_RUN = 0
FAILURES = []

def do_output_test(*args):
    import output_test
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

do_output_test("circa/assert.ca")
do_output_test("circa/boolean.ca")
do_output_test("circa/if.ca")
do_output_test("circa/infix.ca")
do_output_test("circa/lessthan.ca")
do_output_test("circa/subroutine.ca")

print_results()
