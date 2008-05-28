
class CircaFunction(object):
    def __init__(self):
        # inputTypes: list of Terms
        self.inputTypes = []

        # outputType: Term
        self.outputType = None

        # feedbackFunction: Term
        self.feedbackFunction = None

        self.pureFunction = False
        self.hasState = False
        self.name = None

        # evaluateFunc: function that takes a Term
        self.evaluateFunc = None

def initializeTerm(term):
    term.cachedValue = CircaFunction()
def toString(term):
    return "<Function %s>" % term.cachedValue.name

# Accessors
def inputTypes(term):
    return term.cachedValue.inputTypes
def outputType(term):
    return term.cachedValue.outputType
def feedbackFunction(term):
    return term.cachedValue.feedbackFunction
    
# Setters
def setName(term, name):
    term.cachedValue.name = name
def setInputType(term, index, type):
    term.cachedValue.inputTypes = type
def setOutputType(term, type):
    term.cachedValue.outputType = type
def setEvaluateFunc(term, func):
    term.cachedValue.evaluateFunc = func
