import pdb

from circa.parser.token import token_stream
from circa.parser.expression import expression
from circa.parser.token_definitions import *

# test tokenizer
tokens = token_stream("1 2.0")


assert tokens.next().match == INTEGER
assert tokens.next(1).match == FLOAT

assert tokens.consume().match == INTEGER

assert tokens.next().match == FLOAT
assert tokens.consume().match == FLOAT


def parse_to_ast(string):
  tokens = token_stream(string)
  return expression(tokens)

def parse_and_print(string):
  print string + " parses to: " + str(parse_to_ast(string))

parse_and_print("1")

parse_and_print("1 + 2")

parse_and_print("a")
parse_and_print("a=1")
parse_and_print("a = 1 + 2")
