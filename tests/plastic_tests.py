#!/usr/bin/env python

import os, subprocess, sys
from scripts import test_suite

def run_execution_test(filename):
    """
    An execution test will just run the given script and verify that it runs
    without error. It uses the -tr command-line flag.
    """

    result = subprocess.call(["plas","-tr",filename],
        executable=os.environ["CIRCA_HOME"]+'/build/bin/plas')

    if result != 0:
        return ""
    else:
        return True

suite = []

def do_file(filename):
    suite.append(test_suite.TestCase(filename, lambda:run_execution_test(filename)))
        
do_file('plastic/demos/asteroids.ca')
do_file('plastic/demos/buttons.ca')
do_file('plastic/demos/invaders.ca')
do_file('plastic/demos/mouse.ca')
do_file('plastic/demos/pong.ca')

if __name__=='__main__':
    test_suite.run_and_print_results(suite)
