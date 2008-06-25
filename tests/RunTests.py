
import os

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
                raise InputDidntMatch("Expected: %s\nActual:   %s" % (line, stdout_line))

            numLines += 1

        errors = []
    except InputDidntMatch, e:
        print e

    print "Ran \"%s\", %i lines OK" % (command, numLines)
     

stdoutTest('circa if_test.ca', expectedOutput=
"""Reading file if_test.ca...
Simple 1
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
"""Reading file subroutine_test.ca...
Called print_success
Four squared is 16
Five squared is 25
""")

stdoutTest('circa math_test.ca', expectedOutput=
"""Reading file math_test.ca...
one plus two is 3
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
"""Reading file map_test.ca...
should be 3: 3
should be 1: 1
should be 5: 5
should be apple: apple
""")
