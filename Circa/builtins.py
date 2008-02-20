
import string

from Circa import (
  code,
  ca_function
)

VERBOSE_DEBUGGING = False

CODE_UNIT = code.CodeUnit()

# Import all functions from builtin_functions

import builtin_functions

for name in dir(builtin_functions):
  obj = getattr(builtin_functions, name)
  name = string.lower(name)   # lower-case the name
  if isinstance(obj, ca_function.BaseFunction):

    if VERBOSE_DEBUGGING:
      print "Adding builtin function " + name

    # Wrap into a constant term
    CODE_UNIT.createConstant(value=obj, name=name)

# Import some constants
CODE_UNIT.createConstant(value=True, name='true')
CODE_UNIT.createConstant(value=False, name='false')




