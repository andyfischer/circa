
import readline
import cmd, sys
from ctypes import *

libcirca = cdll.LoadLibrary("lib/libcirca.dylib")
libcirca.print_raw_branch.restype = c_char_p
libcirca.print_raw_term.restype = c_char_p
libcirca.term_num_inputs.restype = c_int
libcirca.initialize()

def error(s):
    print "error:",s

def print_term(term):
    return str(libcirca.print_raw_term(term))

class Branch(object):
    def __init__(self, ptr=None):
        self.ptr = ptr
    def find_named(self, name):
        return libcirca.find_named(self.ptr, name)
    def print_raw(self):
        return libcirca.print_raw_branch(self.ptr)

def evaluate_file(filename):
    ptr = libcirca.new_branch()
    libcirca.evaluate_file(ptr, filename)
    return Branch(ptr)

class CommandLine(cmd.Cmd):
    def __init__(self):
        cmd.Cmd.__init__(self)
        self.prompt = "> "
        self.branch = None
        self.termTargetStack = []

    def termTarget(self):
        if self.termTargetStack is []: return None
        else: return self.termTargetStack[-1]

    def pushTarget(self, term):
        print "cursor:", print_term(term)
        self.termTargetStack.append(term)

    def do_open(self, line):
        self.branch = evaluate_file(line)
        print "opened:", line

    def do_t(self, line):
        if line[0] == '#':
            error("unimp")
        else:
            name = line
            t = self.branch.find_named(name)
            self.pushTarget(t)

    def do_input(self, line):
        i = int(line)
        t = libcirca.term_get_input(self.termTarget(), i)
        self.pushTarget(t)

    def do_show(self, line):
        if self.branch is None:
            print "(no branch loaded)"
        else:
            print self.branch.print_raw()

    def do_down(self, line):
        term = self.branch.find_named(line)

    def do_num_inputs(self, line):
        print libcirca.term_num_inputs(self.termTarget())


cl = CommandLine()

if len(sys.argv) > 1:
    cl.do_open(sys.argv[1])

try:
    cl.cmdloop()
except KeyboardInterrupt:
    print ""
