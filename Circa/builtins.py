
import string

from Circa import (
  code,
  ca_function,
  pythonTypes
)

VERBOSE_DEBUGGING = False

BUILTINS = code.CodeUnit()

# Create basic types
INT_TYPE = BUILTINS.createConstant(name = 'int')
FLOAT_TYPE = BUILTINS.createConstant(name = 'float')
STR_TYPE = BUILTINS.createConstant(name = 'str')
BOOL_TYPE = BUILTINS.createConstant(name = 'bool')
FUNCTION_TYPE = BUILTINS.createConstant(name = 'Function')
SUBROUTINE_TYPE = BUILTINS.createConstant(name = 'Subroutine')

# Create basic constants
BUILTINS.createConstant(value=True, name='true')
BUILTINS.createConstant(value=False, name='false')

# Register these with the pythonTypes module
pythonTypes.PYTHON_TO_CIRCA[int] = INT_TYPE
pythonTypes.PYTHON_TO_CIRCA[float] = FLOAT_TYPE
pythonTypes.PYTHON_TO_CIRCA[str] = STR_TYPE
pythonTypes.PYTHON_TO_CIRCA[bool] = BOOL_TYPE

"""
# Import all functions from builtin_functions
import builtin_functions

for name in dir(builtin_functions):
  obj = getattr(builtin_functions, name)
  name = string.lower(name)   # lower-case the name
  if isinstance(obj, ca_function.BaseFunction):

    if VERBOSE_DEBUGGING:
      print "Adding builtin function " + name

    # Wrap into a constant term
    BUILTINS.createConstant(value=obj, name=name)
"""





