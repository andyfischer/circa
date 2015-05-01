#!/usr/bin/env python3

import os,subprocess,sys,re
import itertools
import traceback
from glob import glob

# Path for the circa binary to use in testing. This can be overridden by command-line args.
ExecutablePath = 'build/circa_d -path modules'

# Location of test scripts
TestRoot = 'tests'

Quiet = False
OnlyPrintCommands = False
DumpStats = False
TestSourceRepro = False

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
            
        self.stdin.write((cmd + "\n").encode('utf-8'))

        while True:
            line = self.stdout.readline()
            if not line or line == b":done\n":
                return
            yield line.decode('utf-8')[:-1]

    def kill(self):
        if self.proc is not None:
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

def diff_command_against_file(process, command, filename):
    """
    Run the command 'command' as a separate process, and read from stdin. Also,
    open the file 'file'. Compare the results of 'command' against the contents
    of 'file', and assert that they are the same.

    If the results are the same, we print None. If there is a difference, we return
    an instance of OutputDifference.
    """

    if OnlyPrintCommands:
        print(command)
        return

    if not Quiet:
        print(command)

    if not os.path.exists(filename):
        expectedOutput = []
    else:
        expectedOutput = (line[:-1] for line in open(filename))

    numLines = 0

    try:
        actualOutput = list(process.run(command))
    except Exception as e:
        process.kill()
        raise e

    for actualLine,expectedLine in itertools.zip_longest(actualOutput,expectedOutput, fillvalue=""):
        if expectedLine != actualLine:
            print("\n".join(actualOutput))
            return OutputDifference(actualLine, expectedLine, numLines+1)
        numLines += 1

    return None

def test_file(process, filename):
    failures = []

    # Diff test
    try:
        diff = diff_command_against_file(process, filename, filename + ".output")
    except Exception as e:
        traceback.print_exc()
        process.kill()
        failures.append(TestFailure(["Exception running file"], filename))
        return failures

    if diff:
        desc = ['Script output differed on line '+str(diff.lineNumber)]
        desc.append('  Expected: "'+diff.fromFile+'"')
        desc.append('  Observed: "'+diff.fromCommand+'"')
        failures.append(TestFailure(desc, filename))
        process.kill()

    if TestSourceRepro:
        try:
            diff = diff_command_against_file(process, "-source-repro "+filename, filename)
        except Exception as e:
            traceback.print_exc()
            process.kill()
            failures.append(TestFailure(["Exception running source repro"], filename))
            return failures

        if diff:
            desc = ['Source repro failed on line '+str(diff.lineNumber)]
            desc.append(' Expected: '+diff.fromFile)
            desc.append(' Observed: '+diff.fromCommand)
            failures.append(TestFailure(desc, filename))
            process.kill()

    return failures

def list_files_recr(dir):
    for root, _, files in os.walk(dir):
        for file in files:
            yield os.path.join(root,file)

def list_directory_contents(dir):
    return [os.path.join(dir, f) for f in os.listdir(dir)]

class Suite(object):
    def __init__(self):
        self.totalCount = 0
        self.failedTests = []
        self.totalDisabled = 0
        self.process = CircaProcess()

def run_one_test(suite, file):
    suite.totalCount += 1

    failed = False
    try:
        failures = test_file(suite.process, file)
        if failures:
            failed = True
            print(str(len(failures)) + " failure(s) in "+file)
        for failure in failures:
            for line in failure.description:
                print(" "+line)
    except Exception as e:
        print("Exception occurred during test:", file)
        import traceback
        traceback.print_exc()
        failed = True

    if failed:
        suite.failedTests.append(file)

def run_all_tests(suite):

    # Fetch list of disabled tests
    disabled_test_patterns = []
    for line in open(TestRoot+'/_disabled_tests'):
        line = line.strip()
        line = os.path.join(TestRoot, line)
        #print("skip pattern:", line)
        disabled_test_patterns.append(re.compile(line))


    # Iterate through each test file
    for file in list_files_recr(TestRoot):
        if not file.endswith('.ca'):
            continue

        skip = False
        for pat in disabled_test_patterns:
            if pat.match(file):
                #print('skipping:', file)
                suite.totalDisabled += 1
                skip = True
                break

        if skip:
            continue

        run_one_test(suite, file)

    if OnlyPrintCommands:
        return

def accept_output_for_test(file):
    outfile = file + '.output'
    command = file

    print('[Accepting output for: '+command+']')

    try:
        existingFile = open(outfile, 'r')

        print('[Previous output was..]')
        for line in existingFile:
            print(line.strip())
        print()
    except:
        pass

    print('[Saving results..]')

    out = open(outfile, 'w')

    process = CircaProcess()
    
    for line in process.run(command):
        if not Quiet:
            print(line)
        out.write(line + '\n')

if __name__ == '__main__':
    import optparse
    parser = optparse.OptionParser()
    parser.add_option('--accept', dest="accept")
    parser.add_option('-b', '--binary', dest="binaryPath")
    parser.add_option('--stats', action="store_true", dest="dumpStats")
    parser.add_option('--source-repro', action="store_true", dest="testSourceRepro")
    parser.add_option('--only-print-commands', action="store_true",
        dest="only_print_commands")

    (options,args) = parser.parse_args()

    OnlyPrintCommands = options.only_print_commands
    TestSourceRepro = options.testSourceRepro

    if options.binaryPath:
        ExecutablePath = options.binaryPath

    if options.dumpStats:
        DumpStats = True

    if OnlyPrintCommands:
        Quiet = True

    if options.accept:
        accept_output_for_test(options.accept)
        exit(0)

    if 'CIRCA_HOME' in os.environ:
        os.chdir(os.environ['CIRCA_HOME'])

    suite = Suite()

    if args:
        for arg in args:
            run_one_test(suite, arg)
    else:
        run_all_tests(suite)

    if not OnlyPrintCommands:
        print("Ran",suite.totalCount,"tests,",len(suite.failedTests),"failed,",suite.totalDisabled,"disabled.")

    if DumpStats:
        for line in process.run("dump_stats"):
            print(line)
    
    if suite.failedTests:
        print("Failed tests:")
        for test in suite.failedTests:
            print("  ", test)

        exit(-1)
