#!/usr/bin/env python

import plastic_tests, source_repro, output_tests
import test_helper

suite = []
suite += plastic_tests.suite
suite += source_repro.suite
suite += output_tests.suite

if __name__=='__main__':
    test_helper.run_tests_and_print_results(suite)
