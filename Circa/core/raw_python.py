
from Circa.utils.string_buffer import StringBuffer
import Circa.debug

class RawPythonFunction(object):
    name = 'RawPythonFunction'

    def __init__(self):
        self.function = None

    def compile(funcName, inputs, body):
        debug._assertType(funcName, str)
        debug._assertType(inputs, list)
        debug._assertType(body, str)

        self.funcName = funcName
        self.inputs = inputs
        self.body = body

        # Using funcName, inputs, and body, generate the full python function
        buffer = StringBuffer()
        buffer.writeln("def " + funcName + "(" + ",".join(inputs) + "):")
        buffer.indent()
        buffer.writeln(body)
        self.generatedCode = str(buffer)

        codeObj = compile(self.generatedCode, '<raw-function>', 'exec')

        # Run the code obj, this will create a function in our scope
        exec codeObj

        # Save the function
        self.function = locals()[funcName]

    def call(self, *inputs):
        return self.function(*inputs)
