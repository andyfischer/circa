
import os

def testSame(expected, actual):
    """
    Does a line-wise comparison of the strings expected and actual.
    Returns None if succeeded, or a error message (string) if failed.
    """

    expected = expected.split('\n')
    actual = actual.split('\n')
    for index in range(len(expected)):
        expectedLine = expected[index]
        actualLine = actual[index]

        if expectedLine != actualLine:
            return ("On line %i\nExpected: %s\nOutput:   %s" % 
                    (index, expectedLine, actualLine))

    return None

def runCommand(command):
    """
    Run the given command, return everything that was printed to stdout.
    """
    (stdin, stdout, stderr) = os.popen3(command)

    lines = []

    while True:
        stdout_line = stdout.readline()

        if stdout_line == "":
            break

        # Remove trailing newline
        stdout_line = stdout_line[:-1]

        lines.append(stdout_line)

    return '\n'.join(lines)
