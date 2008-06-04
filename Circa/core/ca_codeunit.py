"""
Define the CodeUnit class
"""
import itertools, pdb

import builtins, ca_function, ca_type
from Circa import debug
from term import Term

class CodeUnit(object):
    def __init__(self):
        # allTerms is a list of Terms
        self.allTerms = []

        # mainBranch is a list of Terms
        self.mainBranch = []

        # mainNamespace maps from strings to Terms
        self.mainNamespace = {}

    def _newTerm(self):
        "Internal method. Returns a new Term object."
        new_term = Term()
        self.allTerms.append(new_term)
        return new_term

    def _bootstrapEmptyTerm(self):
        """
        This function returns a new, empty term. It should only be used
        during the bootstrapping process. In other situations, use
        createTerm instead.
        """
        return self._newTerm()

    def createTerm(self, function, inputs):
        """
        Create a term with the given function and inputs.
        This call may reuse an existing term, if it's correct to do so.
        """

        debug._assert(isinstance(function, Term))

        # Check if they have provided the correct number of arguments
        if len(inputs) != len(ca_function.inputTypes(function)):
            raise Exception("%s() takes %d arguments (%d given)"
                    % (ca_function.name(function),
                       len(ca_function.inputTypes(function)),
                       len(inputs)))

        # Todo: Check if types match

        # Try to reuse an existing term
        if not ca_function.hasState(function):
            existing = findExistingEquivalent(function, inputs)
            if existing:
                return existing

        newTerm = self._newTerm()
        newTerm.functionTerm = function
        newTerm.inputs = list(inputs)

        outputType = ca_function.outputType(function)

        # Initialize cachedData
        debug._assert(outputType is not None)

        initializeFunc = ca_type.initializeFunc(outputType)

        debug._assert(initializeFunc is not None)

        initializeFunc(newTerm)

        # Initialize the term, if this function has an initializeFunc
        if ca_function.initializeFunc(function):
            ca_function.initializeFunc(function)(newTerm)

        newTerm.update()

        # Add ourselves to .users of all our inputs and function
        function.users.add(newTerm)
        for input in inputs:
            input.users.add(newTerm)

        return newTerm

    def createConstant(self, type):
        debug._assert(isinstance(type, Term))

        # Fetch the constant function for this type
        constantFunc = self.createTerm(builtins.CONST_GENERATOR, [type])

        # Create the term
        return self.createTerm(constantFunc, [])

    def bindName(self, term, name, allowOverwrite=False):
        debug._assert(isinstance(name, str))
        debug._assert(isinstance(term, Term))
        if not allowOverwrite and name in self.mainNamespace:
            raise Exception("The name "+name+" is already bound. (to allow "
                + "this, you can use the allowOverwrite parameter)")

        self.mainNamespace[name] = term

    def getNamed(self, name):
        debug._assert(isinstance(name,str))

        try:
            return self.mainNamespace[name]
        except KeyError:
            return None

    def getIdentifier(self, term):
        debug._assert(isinstance(term, Term))
        # Try to find this term in mainNamespace
        for (name,t) in self.mainNamespace.items():
            if t is term:
                return name

        # Otherwise, use the default identifier
        return '#' + str(term.globalID)

    def setInput(self, term, inputIndex, newInput):
        # Grow input array if necessary
        if newInput >= len(term.inputs):
            term.inputs.extend(itertools.repeat(None, inputIndex+1-len(term.inputs)))
        term.inputs[inputIndex] = newInput
        term.needsUpdate = True

    def updateAll(self):
        for term in self.allTerms:
            term.update()

    def _recalculateAllUserSets(self):
        for term in self.allTerms:
            term.users = set()

        for term in self.allTerms:
            term.functionTerm.users.add(term)
            for input in term.inputs:
                input.users.add(term)

def findExistingEquivalent(function, inputs):
    """
    This function finds an existing term that uses the given function,
    and has the given inputs. Returns None if none found.
    """
  
    debug._assert(isinstance(function, Term))

    # Try to find an existing term
    for input in inputs:
        debug._assert(isinstance(input,Term))
    
        for potentialMatch in input.users:
            # Disqualify if they aren't using the same function
            if potentialMatch.functionTerm is not function:
                continue
      
            # Disqualify if inputs don't match
            inputsMatch = True
            for inputIndex in range(len(inputs)):
                if inputs[inputIndex] is not potentialMatch.getInput(inputIndex):
                    inputsMatch = False

            if not inputsMatch:
                continue
                    
            # Accept
            return potentialMatch
   
    # None found
    return None
