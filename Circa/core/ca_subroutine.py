
from Circa.common import debug
from Circa.core import (ca_codeunit, term)

class CircaSubroutine(object):
    def __init__(self):
        self.name = "undefined"
        self.codeUnit = ca_codeunit.CodeUnit()
        self.inputTypes = []
        self.outputType = None

        self.inputPlaceholders = []
        self.outputPlaceholder = None

# Functions for our ca_type object #
def initializeTerm(term):
    term.cachedValue = CircaSubroutine()

def toString(term):
    if term.cachedValue is None:
        return "<undefined Subroutine>"

    return "<Subroutine %s>" % name(term)

def field(fieldName):
    def accessor(term):
        debug._assert(isinstance(term.cachedValue, CircaSubroutine))
        return getattr(term.cachedValue, fieldName)

    def setter(term, value):
        debug._assert(isinstance(term.cachedValue, CircaSubroutine))
        setattr(term.cachedValue, fieldName, value)

    return (accessor, setter)

# Field accessors and setters #
(name, setName) = field('name')
(inputTypes, setInputTypes) = field('inputTypes')
(outputType, setOutputType) = field('outputType')
(inputPlaceholders, setInputPlaceholders) = field('inputPlaceholders')
(outputPlaceholder, setOutputPlaceholder) = field('outputPlaceholder')
(codeUnit, _) = field('codeUnit')

