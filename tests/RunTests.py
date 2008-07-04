
import os

TESTS_PASSED = 0
TESTS_FAILED = 0

class InputDidntMatch(Exception):
    pass

def stdoutTest(command, expectedOutput):
    numLines = 0
    try:
        (stdin, stdout, stderr) = os.popen3(command)

        for line in expectedOutput.split('\n'):
            stdout_line = stdout.readline()

            # Remove the trailing newline
            stdout_line = stdout_line[:-1]

            if line != stdout_line:
                raise InputDidntMatch("Expected: %s\nOutput:   %s" % (line, stdout_line))

            numLines += 1

        errors = []
    except InputDidntMatch, e:
        print "Failed at line", numLines, "while running \"%s\"" % command
        print e
        global TESTS_FAILED
        TESTS_FAILED += 1
        return

    print "Ran \"%s\", %i lines OK" % (command, numLines)
    global TESTS_PASSED
    TESTS_PASSED += 1

def printOverallResults():
    if TESTS_FAILED == 0:
        print "All tests passed (%i)" % TESTS_PASSED
    else:
        def pluralTests(count):
            if count == 1:
                return "1 test"
            else:
                return "%i tests" % count
        print ("Test failures! %s passed. %s failed."
                % (pluralTests(TESTS_PASSED), pluralTests(TESTS_FAILED)))
     

stdoutTest('circa if_test.ca', expectedOutput=
"""Simple 1
Complex condition 1
Else 1
Else 2
Multiline 1
Multiline 2
Multiline 3
Syntax 1
Syntax 2
Syntax 3
Syntax 4
Syntax 5
Syntax 6
Syntax 7
Single line 1
Single line 2
Single line 3
Nested 1
Nested 2
Nested 3
Nested 4
Nested 5
Nested 6
Should be two: 2
Should be three: 3
Should be six: 6
""")

stdoutTest('circa subroutine_test.ca', expectedOutput=
"""Called print_success
Four squared is 16
Five squared is 25
""")

stdoutTest('circa math_test.ca', expectedOutput=
"""one plus two is 3
5.5 plus 2.2 is 7.7
two minus one is 1
one minus two is -1
two times three is 6
eight divided by four is 2
1 + 2 * 3 = 7
1 * 2 + 3 = 5
5 - 4 + 3 = 4
1 - 2 * 3 = -5
1 + 4 / 2 = 9
1 - 4 / 2 = -7
""")

stdoutTest('circa map_test.ca', expectedOutput=
"""should be 3: 3
should be 1: 1
should be 5: 5
should be apple: apple
""")

stdoutTest('circa comparison_test.ca', expectedOutput=
"""5 > 3
3 > 5
1 + 2 > 2
1 + 2 >= 2
1 + 2 >= 3
3 + 6 >= 10
3 + 6 >= 9
""")

stdoutTest('circa boolean_test.ca', expectedOutput=
"""should be true: True
should be false: False
and(true,true): True
and(true,false): False
and(false,true): False
and(false,false): False
or(true,true): True
or(true,false): True
or(false,true): True
or(false,false): False
""")

stdoutTest('circa multiline_test.ca', expectedOutput=
"""
 this
  is
  a
multiline
 string
 test
""")

stdoutTest('circa to-source.ca', expectedOutput=
"""1 + 2
3 * 4 - 2
add(1,2)
average(4,3,2,1)
add(3 * 4,4 / 2)
my-name = to-source(1 + 2)
""")

stdoutTest('circa raw-python.ca', expectedOutput=
"""6
""")

printOverallResults()
