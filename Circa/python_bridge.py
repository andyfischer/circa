
import ca_function

def wrapPythonFunction(func):
    circaFunc = ca_function.Function()
    def funcForCirca(term):
        try:
            term.pythonValue = func(*map(lambda t:t.pythonValue, term.inputs))
        except:
            pass
    circaFunc.pythonEvaluate(funcForCirca)
    return circaFunc

