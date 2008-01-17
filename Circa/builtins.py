
import string

import ca_function
import terms


# Build dict of functions

FUNCTIONS = {}

# Import all functions from builtin_function_defs

import builtin_function_defs

for name in dir(builtin_function_defs):
  obj = getattr(builtin_function_defs, name)
  name = string.lower(name)   # lower-case the name
  if isinstance(obj, ca_function.BaseFunction):
    # Wrap into a constant term
    FUNCTIONS[name] = terms.constant(obj)

# Build dict of constants
CONSTANTS = {}

CONSTANTS['true'] = terms.constant(True)
CONSTANTS['false'] = terms.constant(False)


# Combine symbols
ALL_SYMBOLS = {}

for name in FUNCTIONS:
  ALL_SYMBOLS[name] = FUNCTIONS[name]

for name in CONSTANTS:
  ALL_SYMBOLS[name] = CONSTANTS[name]

# Make sure that ALL_SYMBOLS has types String -> Term
for name in ALL_SYMBOLS:
  if not isinstance(name, str):
    raise AssertionError("Not a string: " + str(name))
  obj = ALL_SYMBOLS[name]
  if not isinstance(obj, terms.Term):
    raise AssertionError("Not a term: " + str(obj))


