
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
builtins.FUNC_TYPE = builtins.BUILTINS.createConstant(name = 'Function',
    value = ca_type.Type(),
    type=builtins.TYPE_TYPE)


# Implant types into 'constant' function
builtins.CONST_FUNC.pythonValue.inputTypes = [builtins.TYPE_TYPE]
builtins.CONST_FUNC.pythonValue.outputTypes = [builtins.FUNC_TYPE]


# Create basic types
builtins.INT_TYPE = builtins.BUILTINS.createConstant(
    name = 'int',
    value = ca_type.Type(),
    type=builtins.TYPE_TYPE)
builtins.FLOAT_TYPE = builtins.BUILTINS.createConstant(name = 'float',
    value = ca_type.Type(),
    type=builtins.TYPE_TYPE)
builtins.STR_TYPE = builtins.BUILTINS.createConstant(name = 'str',
    value = ca_type.Type(),
    type=builtins.TYPE_TYPE)
builtins.BOOL_TYPE = builtins.BUILTINS.createConstant(name = 'bool',
    value = ca_type.Type(),
    type=builtins.TYPE_TYPE)
builtins.SUBROUTINE_TYPE = builtins.BUILTINS.createConstant(name = 'Subroutine',
    value = ca_type.Type(),
    type=builtins.TYPE_TYPE)

# Register these with the pythonTypes module
pythonTypes.PYTHON_TO_CIRCA[int] = builtins.INT_TYPE
pythonTypes.PYTHON_TO_CIRCA[float] = builtins.FLOAT_TYPE
pythonTypes.PYTHON_TO_CIRCA[str] = builtins.STR_TYPE
pythonTypes.PYTHON_TO_CIRCA[bool] = builtins.BOOL_TYPE
pythonTypes.PYTHON_TO_CIRCA[ca_function.Function] = builtins.FUNC_TYPE

# Create basic constants
builtins.BUILTINS.createConstant(name='true', value=True, type=builtins.BOOL_TYPE)
builtins.BUILTINS.createConstant(name='false', value=False, type=builtins.BOOL_TYPE)

