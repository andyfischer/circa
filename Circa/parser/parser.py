#
# parser.py
#
# High-level parser, turns a string into a Circa code object.
#
# Depends on expression.py for expression parsing

from Circa.core import ca_codeunit
from Circa.parser import ast
import expression as _expression_module
import tokens as _tokens_module
from token_definitions import *
import parse_errors

def parseFile(filename, compilationCU=None):
    """
    Parse the given file.
    Returns tuple: (errors, CodeUnit)
       errors is a list of ParseErrors, or an empty list (if none occured)
       codeUnit is an instance of CodeUnit, or None if errors occured
    """

    fileReader = open(filename, 'r')
    fileContents = fileReader.read()
    fileReader.close()

    tokens = _tokens_module.tokenize(fileContents)

    # Scan the token stream for an instance of UNRECOGNIZED token
    # If found, immediately return with this error
    while not tokens.finished():
        token = tokens.consume()
        if token.match == UNRECOGNIZED:
            return ([parse_errors.UnrecognizedToken(token)], None)

    # Reset token stream
    tokens.reset()

    pstate = ParserState(tokens, ca_codeunit.CodeUnit(),
            compilationCU=compilationCU)

    compilation_unit(pstate)

    return (pstate.errors, pstate.codeUnit)


class ParserState(object):
    def __init__(self, tokenSource, resultCodeUnit, compilationCU=None):
        self.tokens = tokenSource
        self.errors = []
        self.codeUnit = resultCodeUnit

        if compilationCU is None:
            self.compilationCU = resultCodeUnit
        else:
            self.compilationCU = compilationCU


def compilation_unit(pstate):

    try:
        resultAst = _expression_module.statement_list(pstate.tokens)
        resultAst.createTerms(
            ast.CompilationContext(pstate.codeUnit, pstate.compilationCU))

        pstate.codeUnit.ast = resultAst

    except parse_errors.ParseError, e:
        pstate.errors.append(e)


def expression_statement(pstate):
    try:
        mark = pstate.tokens.markLocation()
        resultAst = _expression_module.parseExpression(pstate.tokens)
        term = resultAst.createTerms(
            ast.CompilationContext(pstate.codeUnit, pstate.compilationCU))

        pstate.codeUnit.statementAsts.append(resultAst)

    except _expression_module.MatchFailed, e:
        pstate.tokens.restoreMark(mark)
        raise parse_errors.ExpectedExpression(pstate.tokens.next())

