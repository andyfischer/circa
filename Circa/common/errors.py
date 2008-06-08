
class CircaError(Exception):
    pass

def PureVirtualMethodFail(instance, functionName):
    return CircaError("Class "+instance.__class__.__name__
            +" failed to define method "+functionName)
