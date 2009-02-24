#!/usr/bin/env python

import readline
import cmd, sys
from ctypes import *

libcirca = cdll.LoadLibrary("lib/libcirca.dylib")
libcirca.print_raw_branch.restype = c_char_p
libcirca.term_to_raw_string.restype = c_char_p
libcirca.term_num_inputs.restype = c_int
libcirca.term_get_name.restype = c_char_p
libcirca.to_string.restype = c_char_p
libcirca.initialize()

def error(s):
    print "error:",s

class Term(object):
    def __init__(self, ptr=None):
        self.ptr = ptr
    def get_name(self):
        return str(libcirca.term_get_name(self.ptr))
    def get_input(self, i):
        return Term(libcirca.term_get_input(self.ptr, i))
    def num_inputs(self):
        return libcirca.term_num_inputs(self.ptr)
    def has_inner_branch(self):
        return bool(libcirca.has_inner_branch(self.ptr))
    def get_inner_branch(self):
        return Branch(libcirca.get_inner_branch(self.ptr))
    def print_raw(self):
        return str(libcirca.term_to_raw_string(self.ptr))
    def to_string(self):
        return str(libcirca.to_string(self.ptr))
    def evaluate(self):
        libcirca.evaluate_term(self.ptr)

class Branch(object):
    def __init__(self, ptr=None):
        self.ptr = ptr
    def find_named(self, name):
        return Term(libcirca.find_named(self.ptr, name))
    def print_raw(self):
        return libcirca.print_raw_branch(self.ptr)
    def get_outer_branch(self):
        return Branch(libcirca.get_outer_branch(self.ptr))
    def reload_from_file(self):
        libcirca.reload_branch_from_file(self.ptr)
    def find_term_by_id(self, id):
        return Term(libcirca.find_term_by_id(self.ptr, id))

def evaluate_file(filename):
    ptr = libcirca.new_branch()
    libcirca.evaluate_file(ptr, filename)
    return Branch(ptr)

class CommandLine(cmd.Cmd):
    def __init__(self):
        cmd.Cmd.__init__(self)
        self.prompt = "> "
        self.branch = None
        self.term = None
    def setTermTarget(self, term):
        print "current term:", term.print_raw()
        self.term = term
    def do_open(self, line):
        self.branch = evaluate_file(line)
        print "opened:", line
    def do_t(self, line):
        if line[0] == '#':
            id = int(line[1:])
            self.setTermTarget(self.branch.find_term_by_id(id))
        else:
            name = line
            self.setTermTarget(self.branch.find_named(name))
    def do_input(self, line):
        i = int(line)
        self.setTermTarget(self.term.get_input(i))
    def do_show(self, line):
        if self.branch is None:
            error("no branch loaded")
        else:
            print self.branch.print_raw()
    def do_down(self, line):
        term = self.branch.find_named(line)
        if not term.has_inner_branch():
            error("term doesn't have a local branch")
        else:
            self.branch = term.get_inner_branch()
    def do_up(self, line):
        self.branch = self.branch.get_outer_branch()
    def do_num_inputs(self, line):
        print self.term.num_inputs()
    def do_reload(self, line):
        self.branch.reload_from_file()
        self.term = None
    def do_value(self, line):
        print self.term.to_string()
    def do_evaluate(self, line):
        self.term.evaluate()
        self.do_value("")


cl = CommandLine()

if len(sys.argv) > 1:
    cl.do_open(sys.argv[1])

try:
    cl.cmdloop()
except KeyboardInterrupt:
    print ""
