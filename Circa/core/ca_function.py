
from Circa import debug

class CircaFunction(object):
    def __init__(self):
        # inputTypes: list of Terms
        self.inputTypes = []

        # outputType: Term
        self.outputType = None

        self.pureFunction = True
        self.hasState = False
        self.name = None

        # void initializeFunc(Term)
        self.initializeFunc = None

        # void evaluateFunc(Term)
        self.evaluateFunc = None

        # void feedbackFunc(Term)
        self.feedbackFunc = None

def initializeTerm(term):
    term.cachedValue = CircaFunction()
def toString(term):
    return ("<Function %s, pure=%s, state=%s>"
        % (term.cachedValue.name, term.cachedValue.pureFunction, 
            term.cachedValue.hasState))

# Accessors
def name(term):
    return term.cachedValue.name
def inputTypes(term):
    return term.cachedValue.inputTypes
def outputType(term):
    return term.cachedValue.outputType
def hasState(term):
    return term.cachedValue.hasState
def pureFunction(term):
    return term.cachedValue.pureFunction
def initializeFunc(term):
    return term.cachedValue.initializeFunc
def evaluateFunc(term):
    debug._assert(term.cachedValue.evaluateFunc is not None)
    return term.cachedValue.evaluateFunc
def feedbackFunc(term):
    return term.cachedValue.feedbackFunc
    
# Setters
def setName(term, name):
    term.cachedValue.name = name
def setInputTypes(term, types):
    term.cachedValue.inputTypes = types
def setOutputType(term, type):
    term.cachedValue.outputType = type
def setPureFunction(term, pure):
    term.cachedValue.pureFunction = pure
def setHasState(term, hasState):
    term.cachedValue.hasState = hasState
def setInitializeFunc(term, func):
    term.cachedValue.initializeFunc = func
def setEvaluateFunc(term, func):
    term.cachedValue.evaluateFunc = func
