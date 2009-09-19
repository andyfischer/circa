#!/usr/bin/env python

import os, subprocess

def read_text_file(filename):
    f = open(filename)
    contents = f.read()
    return contents

def write_text_file(filename, contents):
    f = open(filename, 'w')
    f.write(contents)

def run_checks():
    # build
    build_result = subprocess.call("scons", shell=True)
    if build_result != 0:
        return "build fail |"

    run_result = subprocess.call("circa", shell=True)
    if run_result != 0:
        return "test fail  |"

    return True

def should_skip_line(line):
    line = line.strip()
    if line == "": return True
    if line.startswith('//'): return True
    if line.startswith('#include'): return True
    if line.startswith('assert('): return True
    return False

def fatal(msg):
    print 'fatal:',msg
    exit(1)

class LinewiseFileDeleter(object):
    def __init__(self, filename):
        self.filename = filename
        self.originalLines = read_text_file(filename).split('\n')
        self.results = {}

    def numLines(self):
        return len(self.originalLines)

    def delete(self, i):
        result = '\n'.join(self.originalLines[:i] + self.originalLines[i+1:])
        write_text_file(self.filename, result)

    def deletionIterator(self):
        for line in range(self.numLines()-1):
            self.delete(line)
            yield line

        self.finish()

    def finish(self):
        write_text_file(self.filename, '\n'.join(self.originalLines))

    def getOriginalWithAnnotations(self, annotations):
        out = []
        for line in range(self.numLines()):
            annotation = ""
            if line in annotations: annotation = annotations[line]
            out.append(annotation + self.originalLines[line])
        return '\n'.join(out)

def run_on_file(filename):
    lfd = LinewiseFileDeleter(filename)
    annotations = {}
    for i in range(lfd.numLines()-1):
        line = lfd.originalLines[i]
        if should_skip_line(line):
            result = "skipped    | "
        else:
            lfd.delete(i)
            result = run_checks()
            if result is True:
                result = "USELESS?   | "
        annotations[i] = result
    lfd.finish()

    return lfd.getOriginalWithAnnotations(annotations)

if __name__ == "__main__":
    os.chdir(os.environ['CIRCA_HOME'])
    initial_result = run_checks()
    if initial_result is not True:
        fatal("Initial build/run failed")

    print run_on_file("src/introspection.cpp")
