
import pdb

from Circa.core import (ca_function, ca_type)
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
        cxt.out.writeln(functionName + " " + str(term.cachedValue))
    elif term.isVariable():
        cxt.out.writeln(functionName + " " + str(term.cachedValue))
    else:
        pdb.set_trace()
        cxt.out.writeln(functionName + "(...)")


