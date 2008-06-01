
# This module contains source for parsing Circa source code, and creating
# code units (or returning errors)

# Includes: expression parsing, high-level parsing, AST objects, tokenizer, token definitions

import expression, tokens

def parseExpression(string):
    "Parse the string as an expression, and return an AST"
    token_stream = tokens.tokenize(string)
    return expression.parseExpression(token_stream)
