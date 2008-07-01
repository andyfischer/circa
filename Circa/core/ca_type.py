
from Circa.common import debug

def emptyFunction(term):
    pass

class CircaType(object):
    def __init__(self):
        self.name = None
        self.allocateData = None
        self.toShortString = None
        self.toSourceString = None

        def iterateInnerTerms(term):
            return []
        self.iterateInnerTerms = iterateInnerTerms

def CircaType_allocateData():
    return CircaType()

def field(fieldName):
    def accessor(term):
        debug._assert(isinstance(term.cachedValue, CircaType))
        return getattr(term.cachedValue, fieldName)

    def setter(term, value):
        debug._assert(isinstance(term.cachedValue, CircaType))
        setattr(term.cachedValue, fieldName, value)

    return (accessor, setter)
    
(name, setName) = field('name')
(allocateData, setAllocateData) = field('allocateData')
(toShortString, setToShortString) = field('toShortString')
(toSourceString, setToSourceString) = field('toSourceString')
(iterateInnerTerms, setIterateInnerTerms) = field('iterateInnerTerms')
