
from Circa.common import (debug,errors)
from Circa.utils.string_buffer import StringBuffer

class BranchSyntaxInfo(object):
    __slots__ = ['lines']

    def __init__(self):
        self.lines = []

    def append(self, line):
        self.lines.append(line)

    def appendNameBinding(self, term, name):
        self.lines.append(NameBindingLine(term,name))

    def appendReturnStatement(self, term):
        self.lines.append(ReturnStatement(term))

    def __iter__(self):
        for line in self.lines:
            yield line

class NameBindingLine(object):
    def __init__(self, term, name):
        self.term = term
        self.name = name

class ReturnStatement(object):
    def __init__(self, term):
        self.term = term
