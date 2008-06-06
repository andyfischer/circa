#
# parser.py
#
# High-level parser, turns a string into a Circa code object.
#
# Depends on expression.py for expression parsing

from Circa.core import ca_codeunit
import expression as _expression_module
import tokens as _tokens_module
from tokens import *
import parse_errors

def parseFile(filename):
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

    # Todo: check for UNRECOGNIZED_TOKEN

    pstate = ParserState(tokens, ca_codeunit.CodeUnit())

    compilation_unit(pstate)

    return (pstate.errors, pstate.codeUnit)


class ParserState(object):
    def __init__(self, tokenSource, codeUnit):
        self.tokens = tokenSource
        self.errors = []
        self.codeUnit = codeUnit

def compilation_unit(pstate):

    while not pstate.tokens.finished():
        try:
            statement(pstate)
        except parse_errors.ParseError, e:
            pstate.errors.append(e)

            # Drop this token, and the rest of this line
            pstate.tokens.consume()
            pstate.tokens.dropUntil(NEWLINE)


def statement(pstate):
    # Dispatch based on next token
    next_token = pstate.tokens.next()

    if next_token.match == TYPE:
        type_decl(pstate)
    elif next_token.match == IF:
        if_statement(pstate)
    elif next_token.match == FUNCTION:
        function_decl(pstate)
    elif next_token.match == RETURN:
        return_statement(pstate)
    elif next_token.match == RBRACKET:
        close_block(pstate)
    elif next_token.match == NEWLINE:
        new_line(pstate)
    else:
        expression_statement(pstate)

def expression_statement(pstate):
    try:
        mark = pstate.tokens.markLocation()
        result = _expression_module.parseExpression(pstate.tokens)
        term = result.eval(pstate.codeUnit)

    except _expression_module.MatchFailed, e:
        pstate.tokens.restoreMark(mark)
        raise parse_errors.ExpectedExpression(pstate.tokens.next())

def new_line(pstate):
    # Ignore NEWLINE
    pstate.tokens.consume(NEWLINE)

