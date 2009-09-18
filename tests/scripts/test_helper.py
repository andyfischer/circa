
import os, subprocess
from subprocess import PIPE

def run_command(command):
    """
    Run the given command, return everything that was printed to stdout.
    """
    proc = subprocess.Popen(command, shell=True, stdin=PIPE, stdout=PIPE, stderr=PIPE, close_fds=True)
    (stdin, stdout, stderr) = (proc.stdin, proc.stdout, proc.stderr)

    lines = []

    while True:
        stdout_line = stdout.readline()

        if stdout_line == "":
            break

        # Remove trailing newline
        stdout_line = stdout_line[:-1]

        lines.append(stdout_line)

    return '\n'.join(lines)

def load_file(filename):
    f = open(filename)
    contents = f.read()
    f.close()
    return contents

def readFileAsLines(filename):
    contents = load_file(filename)

    def myFilter(line):
        # remove blank lines
        if line == '': return False

        # remove commented lines
        if line[0] == '#': return False

        return True

    return filter(myFilter, contents.split('\n'))

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

    expectedOutput = load_file(filename)

    proc = subprocess.Popen(command, shell=True, stdin=PIPE, stdout=PIPE, stderr=PIPE, close_fds=True)
    (stdin, stdout, stderr) = (proc.stdin, proc.stdout, proc.stderr)

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
    
