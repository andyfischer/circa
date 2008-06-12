
class CircaError(Exception):
    pass

def PureVirtualMethodFail(instance, functionName):
    return CircaError("Class "+instance.__class__.__name__
            +" failed to define method "+functionName)

def TypeCheckFail(argumentName, expectedType):
    return CircaError("Type error; expected "+expectedType.__name__+
            " for \""+argumentName+"\"")

def WrongNumberOfArguments(functionName, expectedNumArgs, givenNumArgs):
    return CircaError("%s() takes %d arguments (%d given)"
                    % (functionName, expectedNumArgs, givenNumArgs))
