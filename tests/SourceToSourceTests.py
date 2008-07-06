
import os
import test_helper


def sourceToSourceTest(filename):
    file = open(filename, 'r')
    original_file = file.read()
    file.close()

    actual_output = test_helper.runCommand("circa module-to-source.ca " + filename)

    result = test_helper.testSame(original_file, actual_output)

    if result is not None:
        print "Source-to-source error in file: " + filename
        print result

sourceToSourceTest("module-to-source.ca")
