
from Circa.common import debug
from Circa.core.term import Term

class CircaFunction(object):
    def __init__(self):
        # inputTypes: list of Terms
        self.inputTypes = []

        # outputType: Term
        self.outputType = None

        self.pureFunction = True
        self.hasState = False
        self.name = None

        # True if this function will accept a variable number of arguments
        self.variableArgs = False

        # void initializeFunc(Term)
        self.initializeFunc = None

        # The evaluate function should probably look at Term's inputs,
        # do something, and then put some result into Term.cachedValue.
        # If the function has 'pureFunction' set to False, then it is
        # welcome to do other things too.
        if not hasattr(self, 'evaluateFunc'):
            self.evaluateFunc = None

        self.feedbackAccumulator = None

        self.feedbackPropagator = None

        self.toSourceSpecialHandler = None

def allocateData():
    return CircaFunction()
def toString(term):
    if term.cachedValue is None:
        return "<undefined Function>"

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
def variableArgs(term):
    return term.cachedValue.variableArgs
def initializeFunc(term):
    return term.cachedValue.initializeFunc
def evaluateFunc(term):
    debug._assert(term.cachedValue.evaluateFunc is not None)
    return term.cachedValue.evaluateFunc
def feedbackAccumulator(term):
    return term.cachedValue.feedbackAccumulator
def feedbackPropagator(term):
    return term.cachedValue.feedbackPropagator
    
# Setters
def setName(term, name):
    term.cachedValue.name = name

def setInputTypes(term, types):
    debug._assert(isinstance(types,list))
    term.cachedValue.inputTypes = types

def setOutputType(term, type):
    debug._assert(isinstance(type,Term))
    term.cachedValue.outputType = type

def setPureFunction(term, pure):
    debug._assert(isinstance(pure,bool))
    term.cachedValue.pureFunction = pure

def setHasState(term, hasState):
    debug._assert(isinstance(hasState,bool))
    term.cachedValue.hasState = hasState
def setVariableArgs(term, variableArgs):
    debug._assert(isinstance(variableArgs,bool))
    term.cachedValue.variableArgs = variableArgs
def setInitializeFunc(term, func):
    term.cachedValue.initializeFunc = func

def setEvaluateFunc(term, func):
    term.cachedValue.evaluateFunc = func

def setFeedbackAccumulator(term, func):
    term.cachedValue.feedbackAccumulator = func
def setFeedbackPropagator(term, func):
    term.cachedValue.feedbackPropagator = func
