#!/usr/bin/env python

import plastic_tests
from scripts import test_suite

suite = plastic_tests.suite

if __name__=='__main__':
    test_suite.run_and_print_results(suite)
