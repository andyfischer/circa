
import string

from Circa import (
  builtins,
  code,
  ca_function,
  pythonTypes,
  signature
)

VERBOSE_DEBUGGING = False

builtins.BUILTINS = code.CodeUnit()

# Create basic types
builtins.INT_TYPE = builtins.BUILTINS.createConstant(name = 'int')
builtins.FLOAT_TYPE = builtins.BUILTINS.createConstant(name = 'float')
builtins.STR_TYPE = builtins.BUILTINS.createConstant(name = 'str')
builtins.BOOL_TYPE = builtins.BUILTINS.createConstant(name = 'bool')
builtins.FUNCTION_TYPE = builtins.BUILTINS.createConstant(name = 'Function')
builtins.SUBROUTINE_TYPE = builtins.BUILTINS.createConstant(name = 'Subroutine')

# Register these with the pythonTypes module
pythonTypes.PYTHON_TO_CIRCA[int] = INT_TYPE
pythonTypes.PYTHON_TO_CIRCA[float] = FLOAT_TYPE
pythonTypes.PYTHON_TO_CIRCA[str] = STR_TYPE
pythonTypes.PYTHON_TO_CIRCA[bool] = BOOL_TYPE

# Create basic constants
builtins.BUILTINS.createConstant(name='true', value=True)
builtins.BUILTINS.createConstant(name='false', value=False)

# Create assign function

# Bind := operator

# Parse builtins.ca, add that stuff to BUILTINS

# Add bodies to defined builtin functions

# The following is old and kept for reference only..

# Create 'operator' function
"""
builtins.OPERATOR_FUNC = builtins.BUILTINS.createConstant(name = 'operator',
    value = ca_function.createFunction(inputs=[builtins.TOKEN_TYPE],
                                       outputs=[builtins.FUNCTION_TYPE]),
    type = builtins.FUNCTION_TYPE)
"""




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





