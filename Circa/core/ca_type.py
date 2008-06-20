
from Circa.common import debug

def emptyFunction(term):
    pass

class CircaType(object):
    def __init__(self):
        self.name = None
        self.initialize = None
        self.toShortString = None

        def iterateInnerTerms(term):
            return []
        self.iterateInnerTerms = iterateInnerTerms

def CircaType_initializeTerm(term):
    term.cachedValue = CircaType()

def field(fieldName):
    def accessor(term):
        debug._assert(isinstance(term.cachedValue, CircaType))
        return getattr(term.cachedValue, fieldName)

    def setter(term, value):
        debug._assert(isinstance(term.cachedValue, CircaType))
        setattr(term.cachedValue, fieldName, value)

    return (accessor, setter)
    
(name, setName) = field('name')
(initialize, setInitialize) = field('initialize')
(toShortString, setToShortString) = field('toShortString')
(iterateInnerTerms, setIterateInnerTerms) = field('iterateInnerTerms')
