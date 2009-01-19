
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

        if index >= len(actual):
            return "On line %i\nExpected %s\nOutput reached EOF" % (index,expectedLine)

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

def loadFile(filename):
    f = open(filename)
    contents = f.read()
    f.close()
    return contents

def readFileAsLines(filename):
    contents = loadFile(filename)

    def filterBlanks(line):
        return line != ''

    return filter(filterBlanks, contents.split('\n'))

def compare_command_against_file(command, filename):
    """
    Run the command 'command' as a separate process, and read from stdin. Also,
    open the file 'file'. Compare the results of 'command' against the contents
    of 'file', and assert that they are the same.

    This function will either return a string (indicating an error), or the object
    True if the command-output and file matches.

    If 'command' prints anything to stderr, then we return that as an error.
    """

    if not os.path.exists(filename):
        return "Couldn't find file: " + filename

    expectedOutput = loadFile(filename)

    (stdin, stdout, stderr) = os.popen3(command)

    numLines = 0
    for line in expectedOutput.split('\n'):
        # Make sure there is nothing in stderr
        stderr_line = stderr.readline()
        if stderr_line != "":
            stderr_line = stderr_line[:-1] # remove trailing newline
            return "While running %s, received error: %s" % (command, stderr_line)

        # Read a line from stdout and check it against expected
        stdout_line = stdout.readline()
        stdout_line = stdout_line[:-1] # remove trailing newline

        if line != stdout_line:
            return ("Unexpected output for %s on line %d:\nExpected: %s\nObserved: %s"
                    % (command, numLines, line, stdout_line))

        numLines += 1

    return True
    
