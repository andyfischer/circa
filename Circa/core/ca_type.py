

class CircaType(object):
    def __init__(self):
        self.name = None
        self.initializeFunc = None
        self.toStringFunc = None

def initializeTerm(term):
    term.cachedValue = CircaType()
def typeToString(term):
    return '<Type ' + term.cachedValue.name + '>' 

# Accessors
def name(term):
    return term.cachedValue.name
def initializeFunc(term):
    return term.cachedValue.initializeFunc
def toStringFunc(term):
    return term.cachedValue.toStringFunc

# Setters
def setName(term, name):
    term.cachedValue.name = name
def setInitializeFunc(term, func):
    term.cachedValue.initializeFunc = func
def setToStringFunc(term, func):
    term.cachedValue.toStringFunc = func
