
from Circa import (
   ca_module,
   ca_function,
   code,
   python_bridge,
   debug
)

def create(target):
   target.BUILTIN_MODULE = ca_module.CircaModule()

   target.BUILTINS = target.BUILTIN_MODULE.global_code_unit

# Aliases for creating terms on target.BUILTINS
   createTerm = target.BUILTINS.createTerm
   createConstant = target.BUILTINS.createConstant

# Create constant-generator function, a function which spits out constant functions.
# This term is termporarily incomplete, since he requires a few circular references.
   target.CONST_FUNC_GENERATOR = code.Term()
   target.CONST_FUNC_GENERATOR.codeUnit = target.BUILTINS
   target.CONST_FUNC_GENERATOR.inputs = [target.CONST_FUNC_GENERATOR]
   target.CONST_FUNC_GENERATOR.functionTerm = target.CONST_FUNC_GENERATOR
   target.CONST_FUNC_GENERATOR.globalID = 1
   ca_function.setValue(target.CONST_FUNC_GENERATOR, name="constant-generator")

# Create 'constant-Type' function
   target.CONST_TYPE_FUNC = createTerm(functionTerm=target.CONST_FUNC_GENERATOR)
   ca_function.setValue(target.CONST_TYPE_FUNC, name="constant-Type")

# Create Type type
   target.TYPE_TYPE = createTerm(functionTerm=target.CONST_TYPE_FUNC, name = 'Type')

# Implant the Type type
   target.CONST_TYPE_FUNC.inputs = [target.TYPE_TYPE]
   ca_function.setValue(target.CONST_FUNC_GENERATOR, inputs=[target.TYPE_TYPE])
   ca_function.setValue(target.CONST_TYPE_FUNC, output=target.TYPE_TYPE)

# Create 'constant-Function' function
   target.CONST_FUNC_FUNC = createTerm(functionTerm=target.CONST_FUNC_GENERATOR)
   ca_function.setValue(target.CONST_FUNC_FUNC, name="constant-Function")

# Create Function type
   target.FUNC_TYPE = createTerm(target.CONST_FUNC_FUNC, name = 'Function')

   target.CONST_FUNC_FUNC.inputs = [target.FUNC_TYPE]

# Implant types into 'constant' function, finish defining it
   class ConstFuncGenerator(python_bridge.PythonFunction):
      inputs= [target.TYPE_TYPE]
      output= target.FUNC_TYPE
      @staticmethod
      def evaluate(term):
         debug.Assert(len(term.inputs) == 1)
         type = term.inputs[0]
         debugName = "constant-" + type.getSomeName()
         ca_function.setValue(term, inputs=[], output=type, name=debugName)

   ca_function.setFromPythonFunction(target.CONST_FUNC_GENERATOR, ConstFuncGenerator)

   target.BUILTINS.reevaluate()

