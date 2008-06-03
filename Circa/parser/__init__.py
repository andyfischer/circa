
# This module contains source for parsing Circa source code, and creating
# code units (or returning errors)

# Includes: expression parsing, high-level parsing, AST objects, tokenizer, token definitions

import pdb

import expression as _expression_module
import tokens as _tokens_module
import parser as _parser_module

def parseExpression(string):
    "Parse the string as an expression, and return an AST"
    token_stream = _tokens_module.tokenize(string)
    return _expression_module.parseExpression(token_stream)

parseFile = _parser_module.parseFile
