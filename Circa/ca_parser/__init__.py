
# This module contains source for parsing Circa source code, and creating
# code units (or returning errors)

# Includes: expression parsing, high-level parsing, AST objects, tokenizer, token definitions

import pdb

import tokens as _tokens_module
import ca_parser as _parser_module

def parseStatement(string):
    "Parse the string as a statement, and return an AST"
    token_stream = _tokens_module.tokenize(string)
    return _parser_module.statement(token_stream)

parseFile = _parser_module.parseFile
