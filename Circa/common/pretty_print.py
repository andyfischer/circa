
import pdb

from Circa.core import (builtins, ca_function, ca_subroutine, ca_type)
from Circa.utils import string_buffer

class PrettyPrintContext(object):
    def __init__(self):
        self.out = string_buffer.StringBuffer()
        self.hideConstants = False

def printCodeUnit(codeUnit):
    cxt = PrettyPrintContext()
    _printCodeUnitContents(cxt, codeUnit)
    return str(cxt.out)

def _printCodeUnitContents(cxt, codeUnit):

    for term in codeUnit.allTerms:
        _printTerm(cxt, term)

def _printTerm(cxt, term):
    names = term.getNames()
    if names:
        cxt.out.write(",".join(names) + " = ")

    functionName = ca_function.name(term.functionTerm)

    if term.isConstant():
        _printTermValue(cxt, term)
    elif term.isVariable():
        cxt.out.writeln(functionName + " " + str(term.cachedValue))
    else:
        cxt.out.writeln(functionName + "(...)")

def _printTermValue(cxt, term):
    if term.getType() is builtins.SUBROUTINE_TYPE:
        cxt.out.write("subroutine " + ca_function.name(term))
        cxt.out.indent()
        for term in term.branch:
            _printTerm(cxt,term)
        cxt.out.unindent()

    else:
        functionName = ca_function.name(term.functionTerm)
        cxt.out.writeln(functionName + " " + str(term.cachedValue))
