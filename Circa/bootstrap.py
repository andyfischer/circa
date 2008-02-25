
import string

from Circa import (
  builtins,
  code,
  ca_type,
  ca_function,
  pythonTypes,
  signature,
  terms
)


VERBOSE_DEBUGGING = False

builtins.BUILTINS = code.CodeUnit()

# Create 'constant' function
builtins.CONST_FUNC = builtins.BUILTINS.createTerm(functionTerm=None, name='constant')
builtins.CONST_FUNC.function = builtins.CONST_FUNC
builtins.CONST_FUNC.pythonValue = ca_function.createFunction(inputs=[], outputs=[])

# Create 'constant-Type' function
builtins.CONST_TYPE_FUNC = builtins.BUILTINS.createTerm(functionTerm=builtins.CONST_FUNC)
builtins.CONST_TYPE_FUNC.pythonValue = ca_function.createFunction(inputs=[], outputs=[])

# Create Type type
builtins.TYPE_TYPE = builtins.BUILTINS.createTerm(functionTerm=builtins.CONST_TYPE_FUNC,
    name = 'Type', initialValue = ca_type.Type())

# Implant the Type type
builtins.CONST_FUNC.pythonValue.inputs=[builtins.TYPE_TYPE]
builtins.CONST_TYPE_FUNC.pythonValue.outputs=[builtins.TYPE_TYPE]

# Create Function type
builtins.FUNC_TYPE = builtins.BUILTINS.createConstant(name = 'Type',
    value = ca_type.Type(),
    type=builtins.TYPE_TYPE)


# Implant types into 'constant' function
builtins.CONST_FUNC.pythonValue.inputTypes = [builtins.TYPE_TYPE]
builtins.CONST_FUNC.pythonValue.outputTypes = [builtins.FUNC_TYPE]


# Create basic types
builtins.INT_TYPE = builtins.BUILTINS.createConstant(name = 'int',
    type=builtins.TYPE_TYPE)
builtins.FLOAT_TYPE = builtins.BUILTINS.createConstant(name = 'float',
    type=builtins.TYPE_TYPE)
builtins.STR_TYPE = builtins.BUILTINS.createConstant(name = 'str',
    type=builtins.TYPE_TYPE)
builtins.BOOL_TYPE = builtins.BUILTINS.createConstant(name = 'bool',
    type=builtins.TYPE_TYPE)
builtins.FUNCTION_TYPE = builtins.BUILTINS.createConstant(name = 'Function',
    type=builtins.TYPE_TYPE)
builtins.SUBROUTINE_TYPE = builtins.BUILTINS.createConstant(name = 'Subroutine',
    type=builtins.TYPE_TYPE)

# Register these with the pythonTypes module
pythonTypes.PYTHON_TO_CIRCA[int] = INT_TYPE
pythonTypes.PYTHON_TO_CIRCA[float] = FLOAT_TYPE
pythonTypes.PYTHON_TO_CIRCA[str] = STR_TYPE
pythonTypes.PYTHON_TO_CIRCA[bool] = BOOL_TYPE

# Create basic constants
builtins.BUILTINS.createConstant(name='true', value=True, type=builtins.BOOL_TYPE)
builtins.BUILTINS.createConstant(name='false', value=False, type=builtins.BOOL_TYPE)

# Create builtin functions

def createFunction(name, inputs, outputs):
  return builtins.BUILTINS.createConstant(name=name,
      value=ca_function.createFunction(inputs=input_types, outputs=output_types))

builtins.ASSIGN_FUNC = createFunction('assign')

# Metalanguage functions
#builtins.
builtins.COND_BRANCH_FUNC = createFunction('cond_branch', inputs=[BOOL_TYPE])
builtins.SIMPLE_BRANCH_FUNC = createFunction('simple_branch', inputs=[BOOL_TYPE])
builtins.EQUALS_FUNC = createFunction('equals', outputs=[BOOL_TYPE])
builtins.NOT_EQUALS_FUNC = createFunction('not_equals', outputs=[BOOL_TYPE])

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





