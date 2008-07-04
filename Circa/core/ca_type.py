
import types
from Circa.common import debug
import raw_python

def emptyFunction(term):
    pass

class CircaType(object):
    def __init__(self):
        self.name = "anon"
        self.allocateData = None
        self.toShortString = None
        self.toSourceString = None
        self.iterateInnerTerms = None

    def __setattr__(self, name, obj):
        if name == "name":
            debug.assertType(obj, str)
        elif name == "allocateData":
            debug.assertNullableType(obj, types.FunctionType)
        elif name == "toShortString":
            debug.assertNullableType(obj, types.FunctionType)
        elif name == "toSourceString":
            debug.assertNullableType(obj, types.FunctionType)
        elif name == "iterateInnerTerms":
            debug.assertNullableType(obj, types.FunctionType)
        elif name == "getField":
            debug.assertNullableType(obj, types.FunctionType)
        else:
            debug._assert(False, "CircaType has no field called \""+name+"\"")


        self.__dict__[name] = obj

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
