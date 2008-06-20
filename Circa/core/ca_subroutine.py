
import pdb

from Circa.common import (debug, function_builder)
from Circa.core import (ca_codeunit, ca_variable, term)
import ca_function

class CircaSubroutine(ca_function.CircaFunction):
    def __init__(self):
        ca_function.CircaFunction.__init__(self)

        self.pureFunction = False

        self.codeUnit = ca_codeunit.CodeUnit()

        self.evaluateFunc = subroutineEvaluateFunc


def subroutineEvaluateFunc(cxt):
    subroutine = cxt.caller().functionTerm.cachedValue

    # Copy inputs to subroutine's input placeholders
    for index in range(cxt.numInputs()):
        inputPlaceholderName = "#input_placeholder" + str(index)
        inputPlaceholder = subroutine.codeUnit.getNamed(inputPlaceholderName)
        debug._assert(inputPlaceholder is not None)
        ca_variable.setValue(inputPlaceholder, cxt.input(index))

    # Execute
    subroutine.codeUnit.execute()

    # Find output placeholder
    outputPlaceholder = subroutine.codeUnit.getNamed("#return_val")

    # Copy output placeholder to this term's output
    if outputPlaceholder:
        cxt.setResult(outputPlaceholder.cachedValue)
    else:
        cxt.setResult(None)

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
(codeUnit, _) = field('codeUnit')


