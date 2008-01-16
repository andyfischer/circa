
import string

import ca_function
import term


# Build dict of functions

FUNCTIONS = {}

# Import all functions from builtin_function_defs

import builtin_function_defs

for name in dir(builtin_function_defs):
  obj = getattr(builtin_function_defs, name)
  name = string.lower(name)   # lower-case the name
  if isinstance(obj, ca_function.BaseFunction):
    # Wrap into a constant term
    FUNCTIONS[name] = term.constant(obj)

# Build dict of constants
CONSTANTS = {}

CONSTANTS['true'] = term.constant(True)
CONSTANTS['false'] = term.constant(False)


# Combine symbols
ALL_SYMBOLS = {}

for name in FUNCTIONS:
  ALL_SYMBOLS[name] = FUNCTIONS[name]

for name in CONSTANTS:
  ALL_SYMBOLS[name] = CONSTANTS[name]
