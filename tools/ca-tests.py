#!/usr/bin/env python

import os,subprocess,sys
import traceback
from glob import glob

# Path for the circa binary to use in testing. This can be overridden by command-line args.
ExecutablePath = 'build/circa_d'

# Location of test scripts
TestRoot = 'tests'

Quiet = False
OnlyPrintCommands = False
DumpStats = False

class CircaProcess:
    def __init__(self):
        self.proc = None

    def run(self, cmd):

        # Create proc if necessary
        if self.proc is None:
            self.proc = subprocess.Popen(ExecutablePath + " -run-stdin",
                shell=True, stdin=subprocess.PIPE,
                stdout=subprocess.PIPE, close_fds=True)

            (self.stdin, self.stdout) = (self.proc.stdin, self.proc.stdout)
            
        self.stdin.write(cmd + "\n")

        while True:
            line = self.stdout.readline()
            if not line or line == ":done\n":
                return
            yield line[:-1]

    def kill(self):
        self.proc.kill()
        self.proc = None


class OutputDifference(object):
    def __init__(self, fromCommand, fromFile, lineNumber):
        self.fromCommand = fromCommand
        self.fromFile = fromFile
        self.lineNumber = lineNumber

class TestFailure(object):
    def __init__(self, description, filename):
        self.description = description
        self.filename = filename

def read_text_file(filename):
    f = open(filename)
    contents = f.read()
    return contents

def read_text_file_as_lines(filename):
    f = open(filename)
    while True:
        line = f.readline()
        if not line:
            return
        yield line[:-1]

def diff_command_against_file(process, command, filename):
    """
    Run the command 'command' as a separate process, and read from stdin. Also,
    open the file 'file'. Compare the results of 'command' against the contents
    of 'file', and assert that they are the same.

    If the results are the same, we print None. If there is a difference, we return
    an instance of OutputDifference.
    """

    if OnlyPrintCommands:
        print command
        return

    if not os.path.exists(filename):
        expectedOutput = []
    else:
        expectedOutput = read_text_file_as_lines(filename)

    numLines = 0
    expectedOutput = expectedOutput.__iter__()

    try:
        actualOutput = list(process.run(command))
    except Exception,e:
        process.kill()
        raise e

    for actualLine in actualOutput:
        expectedLine = ""
        try:
            expectedLine = expectedOutput.next()
        except StopIteration:
            pass

        if expectedLine != actualLine:
            print "\n".join(actualOutput)
            return OutputDifference(actualLine, expectedLine, numLines+1)
        numLines += 1

    return None

def test_file(process, filename):
    failures = []

    # Diff test
    try:
        diff = diff_command_against_file(process, "file "+filename, filename + ".output")
    except Exception,e:
        traceback.print_exc()
        failures.append(TestFailure(["Exception running file"], filename))
        return failures

    if diff:
        desc = ['Script output differed on line '+str(diff.lineNumber)]
        desc.append('  Expected: "'+diff.fromFile+'"')
        desc.append('  Observed: "'+diff.fromCommand+'"')
        failures.append(TestFailure(desc, filename))

    # Source repro test
    try:
        diff = diff_command_against_file(process, "source_repro "+filename, filename)
    except Exception,e:
        traceback.print_exc()
        failures.append(TestFailure(["Exception running source repro"], filename))
        return failures

    if diff:
        desc = ['Source repro failed on line '+str(diff.lineNumber)]
        desc.append(' Expected: '+diff.fromFile)
        desc.append(' Observed: '+diff.fromCommand)
        failures.append(TestFailure(desc, filename))

    return failures

def list_files_recr(dir):
    for root, _, files in os.walk(TestRoot):
        for file in files:
            yield os.path.join(root,file)

def list_directory_contents(dir):
    return [os.path.join(dir, f) for f in os.listdir(dir)]

def all_testable_files():
    dirs = [TestRoot,
        TestRoot + '/bugrepro',
        TestRoot + '/if',
        TestRoot + '/error',
        TestRoot + '/exit_points',
        TestRoot + '/feedback',
        TestRoot + '/field',
        TestRoot + '/for',
        TestRoot + '/funcs',
        TestRoot + '/list',
        TestRoot + '/meta',
        TestRoot + '/methods',
        TestRoot + '/reflection',
        TestRoot + '/runtime_typing',
        TestRoot + '/state',
        TestRoot + '/static_errors',
        TestRoot + '/subroutine',
        TestRoot + '/syntax',
        TestRoot + '/types',
        TestRoot + '/type_inferrence'
        ]
    for dir in dirs:
        for f in list_directory_contents(dir):
            if f.endswith('.ca'):
                yield f

def run_all_tests():

    if 'CIRCA_HOME' in os.environ:
        os.chdir(os.environ['CIRCA_HOME'])

    process = CircaProcess()

    totalTestCount = 0
    failedTests = []
    totalDisabledTests = 0

    # Fetch list of disabled tests
    disabled_tests = [os.path.join(TestRoot,f)
        for f in read_text_file_as_lines(TestRoot+'/_disabled_tests')]

    # Iterate through each test file
    # Future: support test files in subdirectories
    for file in all_testable_files():
        if file in disabled_tests:
            totalDisabledTests += 1
            continue

        totalTestCount += 1

        if not Quiet:
            print file

        failed = False
        try:
            failures = test_file(process, file)
            if failures:
                failed = True
                print str(len(failures)) + " failure(s) in "+file+":"
            for failure in failures:
                for line in failure.description:
                    print " "+line
        except Exception,e:
            print "Exception occurred during test:", file
            import traceback
            traceback.print_exc()
            failed = True

        if failed:
            failedTests.append(file)

    if OnlyPrintCommands:
        return

    print "Ran",totalTestCount,"tests,",len(failedTests),"failed,",totalDisabledTests,"disabled."

    if DumpStats:
        for line in process.run("dump_stats"):
            print line
    
    if failedTests:
        print "Failed tests:"
        for test in failedTests:
            print "  ", test

        exit(-1)

def accept_output_for_test(file):

    outfile = file + '.output'
    command = "file " + file

    print 'Running: '+command
    print 'Saving results to: '+outfile

    out = open(outfile, 'w')

    process = CircaProcess()
    
    for line in process.run(command):
        if not Quiet:
            print line
        out.write(line + '\n')

if __name__ == '__main__':
    import optparse
    parser = optparse.OptionParser()
    parser.add_option('--accept', dest="accept")
    parser.add_option('-b', '--binary', dest="binaryPath")
    parser.add_option('--stats', action="store_true", dest="dumpStats")
    parser.add_option('--only-print-commands', action="store_true",
        dest="only_print_commands")

    (options,args) = parser.parse_args()

    OnlyPrintCommands = options.only_print_commands

    if options.binaryPath:
        ExecutablePath = options.binaryPath

    if options.dumpStats:
        DumpStats = True

    if OnlyPrintCommands:
        Quiet = True

    if options.accept:
        accept_output_for_test(options.accept)
        exit(0)

    run_all_tests()
