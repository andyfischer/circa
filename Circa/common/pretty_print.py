
from Circa.core import (ca_function, ca_type)
from Circa.utils import string_buffer

class PrettyPrintContext(object):
    def __init__(self):
        self.out = string_buffer.StringBuffer()
        self.hideConstants = False

def printCodeUnit(codeUnit):
    cxt = PrettyPrintContext()
    _printCodeUnit(cxt, codeUnit)
    return str(cxt.out)

def _printCodeUnit(cxt, codeUnit):

    #cxt.out.writeln("Code \"%s\":" % codeUnit.name)
    cxt.out.writeln("CodeUnit:")
    cxt.out.indent()

    for term in codeUnit.allTerms:
        _printTerm(cxt, term)

    cxt.out.unindent()

def _printTerm(cxt, term):
    names = term.getNames()
    if names:
        cxt.out.write(",".join(names) + " = ")

    if term.isConstant():
        cxt.out.writeln(term.cachedValue)
    elif term.isVariable():
        cxt.out.writeln("variable " + term.cachedValue)
    else:
        cxt.out.writeln(ca_function.name(term.functionTerm) + "(...)")






