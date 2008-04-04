
import string, os, pdb

from Circa import (
    builtins,
    builtin_functions,
    containers,
    code,
    ca_type,
    ca_function,
    parser,
    python_bridge,
    signature,
    token
)

# fixme
CIRCA_HOME = "/Users/andyfischer/code/circa"

VERBOSE_DEBUGGING = False

builtins.BUILTINS = code.CodeUnit()

# Create 'constant' function, which temporarily does not have a function
builtins.CONST_FUNC = code.Term()
builtins.CONST_FUNC.codeUnit = builtins.BUILTINS
builtins.CONST_FUNC.functionTerm = builtins.CONST_FUNC
builtins.BUILTINS.setTermName(builtins.CONST_FUNC, "constant")
builtins.CONST_FUNC.globalID = 1
builtins.CONST_FUNC.pythonValue = ca_function.Function(inputs=[], output=None)

# Create 'constant-Type' function
builtins.CONST_TYPE_FUNC = builtins.BUILTINS.createTerm(functionTerm=builtins.CONST_FUNC)
builtins.CONST_TYPE_FUNC.pythonValue = ca_function.Function(inputs=[], output=None)

# Create Type type
builtins.TYPE_TYPE = builtins.BUILTINS.createTerm(
    functionTerm=builtins.CONST_TYPE_FUNC,
    name = 'Type', initialValue = ca_type.Type())
builtins.TYPE_TYPE.pythonValue.outputType=builtins.TYPE_TYPE

# Implant the Type type
builtins.CONST_FUNC.pythonValue.inputs=[builtins.TYPE_TYPE]
builtins.CONST_TYPE_FUNC.pythonValue.outputType=builtins.TYPE_TYPE

# Create Function type
builtins.FUNC_TYPE = builtins.BUILTINS.createConstant(name = 'Function',
    value = ca_type.Type(),
    valueType=builtins.TYPE_TYPE)
python_bridge.PYTHON_TYPE_TO_CIRCA[ca_function.Function] = builtins.FUNC_TYPE

# Implant types into 'constant' function
builtins.CONST_FUNC.pythonValue.inputTypes = [builtins.TYPE_TYPE]
builtins.CONST_FUNC.pythonValue.outputType = builtins.FUNC_TYPE

# Create and register primitive types
builtins.INT_TYPE = builtins.BUILTINS.createConstant(name = 'int',
    value = ca_type.Type(),
    valueType=builtins.TYPE_TYPE)
python_bridge.PYTHON_TYPE_TO_CIRCA[int] = builtins.INT_TYPE

builtins.FLOAT_TYPE = builtins.BUILTINS.createConstant(name = 'float',
    value = ca_type.Type(),
    valueType=builtins.TYPE_TYPE)
python_bridge.PYTHON_TYPE_TO_CIRCA[float] = builtins.FLOAT_TYPE

builtins.STR_TYPE = builtins.BUILTINS.createConstant(name = 'string',
    value = ca_type.Type(),
    valueType=builtins.TYPE_TYPE)
python_bridge.PYTHON_TYPE_TO_CIRCA[str] = builtins.STR_TYPE

builtins.BOOL_TYPE = builtins.BUILTINS.createConstant(name = 'bool',
    value = ca_type.Type(),
    valueType=builtins.TYPE_TYPE)
python_bridge.PYTHON_TYPE_TO_CIRCA[bool] = builtins.BOOL_TYPE

builtins.REF_TYPE = builtins.BUILTINS.createConstant(name = 'Ref',
    value = ca_type.Type(),
    valueType=builtins.TYPE_TYPE)

builtins.SUBROUTINE_TYPE = builtins.BUILTINS.createConstant(name = 'Subroutine',
    value = ca_type.Type(),
    valueType=builtins.TYPE_TYPE)
python_bridge.PYTHON_TYPE_TO_CIRCA[code.SubroutineDefinition] = builtins.SUBROUTINE_TYPE

# Create basic constants
builtins.BUILTINS.createConstant(name='true', value=True, valueType=builtins.BOOL_TYPE)
builtins.BUILTINS.createConstant(name='false', value=False, valueType=builtins.BOOL_TYPE)

# Create Map function
mapFunctionObj = ca_function.Function(
    inputs=[builtins.TYPE_TYPE, builtins.TYPE_TYPE],
    output=[builtins.FUNC_TYPE],
    init = builtin_functions.mapGeneratorInit,
    evaluate = builtin_functions.mapGeneratorEval,
    hasState=True)
builtins.MAP_GENERATOR = builtins.BUILTINS.createConstant(name = 'map', value=mapFunctionObj)

mapTrainingObj = ca_function.Function(
      inputs=[builtins.FUNC_TYPE, builtins.REF_TYPE, builtins.REF_TYPE],
      output=None, # pureFunction=False,
      evaluate = builtin_functions.mapTraining)
builtins.MAP_TRAINING_FUNC = builtins.BUILTINS.createConstant(value=mapTrainingObj)
mapFunctionObj.trainingFunc = builtins.MAP_TRAINING_FUNC

# Load builtins.ca file into this code unit
builtinsFilename = os.path.join(CIRCA_HOME, "lib", "builtins.ca")
parser.parseFile(parser.builder.Builder(target=builtins.BUILTINS),
         builtinsFilename, raise_errors=True,
         pythonObjectSource=builtin_functions.NAME_TO_FUNC)

# Expose some objects that were created in Circa code so that they may be accessed
# from Python code

def getCircaDefined(name):
   obj = builtins.BUILTINS.getNamedTerm(name)
   if obj is None: raise Exception("Couldn't find term named: " + name)
   return obj

builtins.TOKEN_FUNC = getCircaDefined("token")
builtins.OPERATOR_FUNC = getCircaDefined("operator")
builtins.ASSIGN_OPERATOR_FUNC = getCircaDefined("assign_operator")

# Install builtin functions into pre-existing Circa objects
def installFunc(name, value):
   targetTerm = builtins.BUILTINS.getNamedTerm(name)

   # Make sure nothing else has been installed
   if targetTerm.pythonValue.pythonEvaluate is not parser.PLACEHOLDER_EVALUATE_FOR_BUILTINS:
      raise Exception("Term " + name + " already has a builtin function installed")

   targetTerm.pythonValue.pythonInit = value.pythonInit
   targetTerm.pythonValue.pythonEvaluate = value.pythonEvaluate


