
import string, os, pdb

from Circa import (
    builtins,
    code,
    ca_type,
    ca_function,
    parser,
    python_bridge,
    signature,
    token
)

# todo
CIRCA_HOME = "/Users/andyfischer/code/circa"

VERBOSE_DEBUGGING = False

builtins.BUILTINS = code.CodeUnit()

# Create 'constant' function, which temporarily does not have a function
builtins.CONST_FUNC = code.Term()
builtins.CONST_FUNC.codeUnit = builtins.BUILTINS
builtins.CONST_FUNC.functionTerm = builtins.CONST_FUNC
builtins.BUILTINS.setTermName(builtins.CONST_FUNC, "constant")
builtins.CONST_FUNC.globalID = 1
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
builtins.INT_TYPE = builtins.BUILTINS.createConstant(name = 'int',
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

# Register these types
python_bridge.PYTHON_TYPE_TO_CIRCA[int] = builtins.INT_TYPE
python_bridge.PYTHON_TYPE_TO_CIRCA[float] = builtins.FLOAT_TYPE
python_bridge.PYTHON_TYPE_TO_CIRCA[str] = builtins.STR_TYPE
python_bridge.PYTHON_TYPE_TO_CIRCA[bool] = builtins.BOOL_TYPE
python_bridge.PYTHON_TYPE_TO_CIRCA[ca_function.Function] = builtins.FUNC_TYPE

# Create basic constants
builtins.BUILTINS.createConstant(name='true', value=True, type=builtins.BOOL_TYPE)
builtins.BUILTINS.createConstant(name='false', value=False, type=builtins.BOOL_TYPE)

# Load builtins.ca file into this code unit
def installLibFile(filename):
    file = open(os.path.join(CIRCA_HOME, "lib", filename), 'r')
    file_contents = file.read()
    file.close()
    del file
    parser.parse(parser.builder.Builder(target=builtins.BUILTINS),
            token.tokenize(file_contents), raise_errors=True)
installLibFile("builtins.ca")

# Access some objects were created in builtins.ca (and which need to be made available
# to Python code)
builtins.TOKEN_FUNC = builtins.BUILTINS.getNamedTerm("token")
builtins.OPERATOR_FUNC = builtins.BUILTINS.getNamedTerm("operator")
builtins.ASSIGN_OPERATOR_FUNC = builtins.BUILTINS.getNamedTerm("operator")

# Fill in definitions for all builtin functions
def installFunc(name, func):
    builtins.BUILTINS.getNamedTerm(name).pythonValue = python_bridge.wrapPythonFunction(func)

def tokenEvaluate(s):
    return token.definitions.STRING_TO_TOKEN[s]
installFunc("token", tokenEvaluate)

def addEvaluate(a,b): return a + b
def subEvaluate(a,b): return a - b
def multEvaluate(a,b): return a * b
def divEvaluate(a,b): return a / b
installFunc("add", addEvaluate)
installFunc("sub", subEvaluate)
installFunc("mult", multEvaluate)
installFunc("div", divEvaluate)

# Read in parsing.ca
installLibFile("parsing.ca")

# pdb.set_trace()
