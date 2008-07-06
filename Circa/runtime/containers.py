
import pdb
from Circa.core import (builtins, ca_function, ca_type)
from Circa.common import (debug, function_builder)

# Create Map functions
class MapConstructor(object):
    name = 'map'
    inputs = ['type','type']
    output = 'Function'
    hasState = True
    pure = True
    meta = True
    instanceBased = True

    def __init__(self):
        self.hashtable = {}

    def evaluate(self, cxt):
        keyType = cxt.inputTerm(0)
        valueType = cxt.inputTerm(1)
        ca_function.setName(cxt.caller(), 'map-' + 
                ca_type.name(keyType) + '-to-'
                + ca_type.name(valueType))
        ca_function.setInputTypes(cxt.caller(), [keyType])
        ca_function.setOutputType(cxt.caller(), valueType)
        ca_function.setPureFunction(cxt.caller(), True)
        ca_function.setEvaluateFunc(cxt.caller(), MapAccess_evaluate)
        ca_function.setFeedbackPropagator(cxt.caller(), MAP_FEEDBACK)

def MapAccess_evaluate(cxt):
    hashtable = cxt.caller().functionTerm.state.hashtable
    key = cxt.input(0)
    try:
        cxt.caller().cachedValue = hashtable[key]
    except KeyError:
        cxt.caller().cachedValue = None

class MapFeedback(object):
    name = 'map-feedback'
    inputs = ['ref','ref']
    output = 'void'
    hasState = False
    pure = False
    meta = True

    @staticmethod
    def evaluate(context):
        target = context.inputTerm(0)
        desired = context.input(1)

        # Goal here is to change target's function so that it outputs desired
        hashtable = target.functionTerm.state.hashtable
        key = target.getInput(0).cachedValue
        hashtable[key] = desired
        target.functionTerm.needsUpdate = True

class ListType(object):
    name = 'List'
    instanceBased = False

    @staticmethod
    def toShortString(term):
        print term.cachedValue

class PackList(object):
    name = 'list'
    inputs = ['ref']
    output = 'List'
    variableArgs = True
    pure = True

    @staticmethod
    def evaluate(*args):
        return args


MAP_FEEDBACK = None

def createFunctions(codeUnit):
    function_builder.importPythonFunction(codeUnit, MapConstructor)

    global MAP_FEEDBACK
    MAP_FEEDBACK = function_builder.importPythonFunction(codeUnit, MapFeedback)

    function_builder.importPythonType(codeUnit, ListType)
    builtins.PACK_LIST_FUNC = function_builder.importPythonFunction(codeUnit, PackList)

