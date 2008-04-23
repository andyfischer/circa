
import string, os, pdb

from Circa import (
    builtins,
    create_kernel,
    containers,
    code,
    ca_function,
    ca_module,
    ca_type,
    debug,
    parser,
    python_bridge,
    signature,
    token
)

# fixme
CIRCA_HOME = "/Users/andyfischer/code/circa"

VERBOSE_DEBUGGING = False

create_kernel.create(target = builtins)

# Aliases for creating terms on builtins.BUILTINS
createTerm = builtins.BUILTINS.createTerm
createConstant = builtins.BUILTINS.createConstant

# Create and register primitive types
builtins.INT_TYPE = createConstant(name = 'int', value = ca_type.Type(),
    valueType=builtins.TYPE_TYPE)

builtins.FLOAT_TYPE = createConstant(name = 'float', value = ca_type.Type(),
    valueType=builtins.TYPE_TYPE)

builtins.STR_TYPE = createConstant(name = 'string', value = ca_type.Type(),
    valueType=builtins.TYPE_TYPE)

builtins.BOOL_TYPE = createConstant(name = 'bool', value = ca_type.Type(),
    valueType=builtins.TYPE_TYPE)

builtins.REF_TYPE = createConstant(name = 'Ref', value = ca_type.Type(),
    valueType=builtins.TYPE_TYPE)

builtins.SUBROUTINE_TYPE = createConstant(name = 'Subroutine',
    value = ca_type.Type(),
    valueType=builtins.TYPE_TYPE)

builtins.BUILTINS.reevaluate()

python_bridge.registerType(int, builtins.INT_TYPE)
python_bridge.registerType(float, builtins.FLOAT_TYPE)
python_bridge.registerType(str, builtins.STR_TYPE)
python_bridge.registerType(bool, builtins.BOOL_TYPE)
python_bridge.registerType(code.SubroutineDefinition, builtins.SUBROUTINE_TYPE)

# Create basic constants
createConstant(name='true', value=True, valueType=builtins.BOOL_TYPE)
createConstant(name='false', value=False, valueType=builtins.BOOL_TYPE)

# Import builtin_functions.py . Can't do this sooner because it requires a bunch
# of symbols that we just defined.
from Circa import builtin_functions

# Create Map function
builtins.MAP_GENERATOR = createConstant(name = 'map',
      valueType=builtins.FUNC_TYPE)

ca_function.setFromPythonFunction(builtins.MAP_GENERATOR, builtin_functions.MapGenerator)

# Create Variable generator function
builtins.VARIABLE_FUNC_GENERATOR = createConstant(valueType=builtins.FUNC_TYPE) 
ca_function.setFromPythonFunction(builtins.VARIABLE_FUNC_GENERATOR,
      builtin_functions.VariableGenerator)

# Create feedback function
builtins.FEEDBACK_FUNC = createConstant(name="feedback", valueType=builtins.FUNC_TYPE)

ca_function.setFromPythonFunction(builtins.FEEDBACK_FUNC, builtin_functions.Feedback)

# Create Unknown function generator
builtins.UNKNOWN_FUNC_GENERATOR = createConstant(valueType=builtins.FUNC_TYPE)
ca_function.setFromPythonFunction(builtins.UNKNOWN_FUNC_GENERATOR,
      builtin_functions.UnknownFunctionGenerator)

builtins.BUILTINS.reevaluate()

# Load builtins.ca file into this code unit
builtinsFilename = os.path.join(CIRCA_HOME, "lib", "builtins.ca")
parser.parseFile(builtins.BUILTIN_MODULE, builtinsFilename, raise_errors=True)

# Go through objects defined in bulitins.ca, and fill in definitions
for (name,obj) in builtins.BUILTINS.term_namespace.items():
   if name in builtin_functions.NAME_TO_FUNC:
      ca_function.setFromPythonFunction(obj, builtin_functions.NAME_TO_FUNC[name])
      obj.evaluate()

builtins.BUILTINS.reevaluate()

# Expose some objects that were created in Circa code so that they may be accessed
# from Python code

def getCircaDefined(name):
   obj = builtins.BUILTINS.getNamedTerm(name)
   if obj is None: raise Exception("Couldn't find term named: " + name)
   return obj

builtins.TOKEN_FUNC = getCircaDefined("token")
builtins.OPERATOR_FUNC = getCircaDefined("operator")
builtins.ASSIGN_OPERATOR_FUNC = getCircaDefined("assign_operator")
builtins.ASSIGN_FUNC = getCircaDefined("assign")
builtins.ADD_FUNC = getCircaDefined("add")
builtins.SUBTRACT_FUNC = getCircaDefined("sub")
builtins.MULTIPLY_FUNC = getCircaDefined("mult")

