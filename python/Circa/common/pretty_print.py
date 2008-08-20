
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

    for term in codeUnit.mainBranch:
        _printTerm(cxt, term)

def printBranch(cxt, branch):
    for term in branch:
        _printTerm(cxt,term)

def _printTerm(cxt, term):
    names = term.getNames()
    if names:
        cxt.out.write(",".join(names) + " = ")
    else:
        cxt.out.write(shortIdentifier(term) + " = ")

    functionName = ca_function.name(term.functionTerm)

    if term.isConstant():
        _printTermValue(cxt, term)
    elif term.isVariable():
        _printTermValue(cxt, term)
    elif term.functionTerm is builtins.IF_STATEMENT:
        cxt.out.writeln("if " + shortIdentifier(term.getInput(0)))
        cxt.out.indent()
        printBranch(cxt, term.state.branches[0])
        cxt.out.unindent()
        if len(term.state.branches) > 1:
            cxt.out.writeln("else")
            cxt.out.indent()
            printBranch(cxt, term.state.branches[1])
            cxt.out.unindent()
    elif term.functionTerm is builtins.WHILE_STATEMENT:
        cxt.out.writeln("while " + shortIdentifier(term.getInput(0)))
        cxt.out.indent()
        printBranch(cxt, term.state.branch)
        cxt.out.unindent()
    elif term.functionTerm is builtins.FEEDBACK_FUNC:
        cxt.out.write(functionName + "(")
        cxt.out.write(', '.join(map(shortIdentifier, term.inputs)))
        cxt.out.writeln(")")
        cxt.out.indent()
        printBranch(cxt, term.state.branch)
        cxt.out.unindent()
    else:
        cxt.out.write(functionName + "(")
        cxt.out.write(', '.join(map(shortIdentifier, term.inputs)))
        cxt.out.writeln(")")

def shortIdentifier(term):
    if term.givenName:
        return term.givenName
    return term.getIdentifier()

def _printTermValue(cxt, term):
    if term.getType() is builtins.SUBROUTINE_TYPE:
        cxt.out.writeln("subroutine " + ca_function.name(term) + ' {')
        cxt.out.indent()
        for term in term.iterateInnerTerms():
            _printTerm(cxt,term)
        cxt.out.unindent()
        cxt.out.writeln('}')

    elif term.isConstant():
        cxt.out.writeln("constant " + str(term))

    elif term.isVariable():
        cxt.out.writeln("variable " + str(term))

    else:
        functionName = ca_function.name(term.functionTerm)
        cxt.out.writeln(functionName + " " + str(term.cachedValue))

def getInputsAsWritten(term):
    """
    Returns a list of strings for each input in term. We attempt (by checking
    the AST) to replicate how each input was originally specified in source.
    """
    for index in term.numInputs():
        inputTerm = term.getInput(index)
        inputExpr = term.ast
        yield str(inputExpr)

