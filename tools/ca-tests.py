#!/usr/bin/env python

import os,subprocess,sys
import traceback
from glob import glob

ExecutableName = 'circa_d'
TestRoot = 'tests'

class CircaProcess:
    def __init__(self):
        self.proc = None

    def run(self, cmd):
        # Create proc if necessary
        if self.proc is None:
            self.proc = subprocess.Popen("circa_d -run-stdin",
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
        TestRoot + '/field',
        TestRoot + '/bugrepro',
        TestRoot + '/for',
        TestRoot + '/static_errors'
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
    totalFailedTests = 0
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
            totalFailedTests += 1


    print "Ran",totalTestCount,"tests,",totalFailedTests,"failed,",totalDisabledTests,"disabled."
    
    if totalFailedTests > 0:
        exit(-1)

def find_test_with_name(name):
    import glob

    # find all files containing the substring 'name'
    files = glob.glob(TestRoot+'/*'+name+'*')
    files = list(files)

    # prefer an exact match
    for file in files:
        if file.endswith(name):
            return file

    # only look at .ca files
    files = filter(lambda f: f.endswith('.ca'), files)
    print files

    try:
        return files[0]
    except IndexError:
        return None

def accept_output_for_test(name):
    file = find_test_with_name(name)
    outfile = file + '.output'
    cmd = ExecutableName+' '+file

    print 'Running: '+cmd
    print 'Saving results to: '+outfile

    out = open(outfile, 'w')
    for line in run_command(cmd):
        print line
        out.write(line + '\n')

if __name__ == '__main__':
    import optparse
    parser = optparse.OptionParser()
    parser.add_option('--accept', dest="accept")

    (options,args) = parser.parse_args()

    if options.accept:
        accept_output_for_test(options.accept)
        exit(0)

    run_all_tests()
