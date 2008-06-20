
from Circa.core import (builtins, ca_type)
from Circa.common import debug

def importPythonType(codeUnit, pythonClass):
    """
    Import the given Python class as a Circa type.
    This class must define the following fields:
      name
      toShortString
      iterateInnerTerms
    """

    debug._assert(isinstance(pythonClass, type))

    typeTerm = codeUnit.createConstant(builtins.TYPE_TYPE)

    ca_type.setName(typeTerm, pythonClass.name)

    def initialize(term):
        term.cachedValue = pythonClass()

    def toShortString(term):
        return term.cachedValue.toShortString()

    def iterateInnerTerms(term):
        return term.cachedValue.iterateInnerTerms()

    ca_type.setInitialize(typeTerm, initialize)
    ca_type.setToShortString(typeTerm, toShortString)
    ca_type.setIterateInnerTerms(typeTerm, iterateInnerTerms)

    codeUnit.bindName(typeTerm, pythonClass.name)

    return typeTerm
