"""
Define the CodeUnit class
"""
import itertools

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

    def createTerm(self, function, inputs, forceCreate=False):

        # Check if we can reuse an existing term
        if not forceCreate and not ca_function.hasState(function):
            existing = findExistingEquivalent(function, inputs)
            if existing:
                return existing

        newTerm = self._newTerm()
        newTerm.functionTerm = function
        newTerm.inputs = list(inputs)

        # Todo: more stuff here

        return newTerm

    def createConstant(self, type):
        debug._assert(isinstance(type, Term))

        # Fetch the constant function for this type
        constantFunc = self.createTerm(builtins.CONST_GENERATOR, [type])

        # Create the term
        return self.createTerm(constantFunc, [], forceCreate=True)

    def bindName(self, term, name, allowOverwrite=False):
        debug._assert(isinstance(name, str))
        debug._assert(isinstance(term, Term))
        if not allowOverwrite and name in self.mainNamespace:
            raise Exception("The name "+name+" is already bound. (to allow "
                + "this, you can use the allowOverwrite parameter)")

        self.mainNamespace[name] = term

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

def findExistingEquivalent(function, inputs):
    """
    This function finds an existing term that uses the given function,
    and has the given inputs. Returns None if none found.
    """

    if inputs is None:
        return None
  
    debug._assert(isinstance(function, Term))
  
    # Try to find an existing term
    for input in inputs:
        debug._assert(isinstance(input,Term))
    
        for potentialMatch in input.users:
            # Disqualify if they aren't using the same function
            if potentialMatch.functionTerm != functionTerm:
                continue
      
            # Disqualify if inputs don't match
            ...
            def matches(pair):
               return pair[0].equals(pair[1])
      
            inputs_match = all(map(matches, zip(inputs, potentialMatch.inputs)))
      
            # Todo: allow for functions that don't care what the function order is
      
            if not inputs_match: continue
      
            # Looks like this term is the same as what they want
            return potentialMatch
   
    return None
     
