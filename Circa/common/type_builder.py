
from Circa.core import (builtins, ca_type)

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

    ca_type.setInitializeFunc(typeTerm, initialize)

    return typeTerm
