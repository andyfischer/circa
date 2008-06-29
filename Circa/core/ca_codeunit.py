"""
Define the CodeUnit class
"""
import itertools, pdb

import Circa
from Circa.common import (debug, errors)
import builtins, ca_function, ca_type
from term import Term
from branch import Branch

class CodeUnit(object):
    def __init__(self):
        # allTerms is a list of Terms
        self.allTerms = []

        # mainBranch is a list of Terms
        self.mainBranch = Branch(None)

        # an AST object that describes this code
        self.ast = None

    def _newTerm(self, branch = None):
        "Internal method. Returns a new Term object."
        new_term = Term()
        new_term.codeUnit = self

        if branch is None:
            branch = self.mainBranch
        else:
            debug._assert(isinstance(branch,Branch))

        branch.append(new_term)
        self.allTerms.append(new_term)
        return new_term

    def _bootstrapEmptyTerm(self):
        """
        This function returns a new, empty term. It should only be used
        during the bootstrapping process. In other situations, use
        createTerm instead.
        """
        return self._newTerm()

    def createTerm(self, function, inputs, branch=None):
        """
        Create a term with the given function and inputs.
        This call may reuse an existing term, if it's correct to do so.

        If 'branch' is specified, create the term inside that branch.
        Otherwise, the term is created in the main branch.
        """

        debug._assert(isinstance(function, Term))
        debug._assert(isinstance(inputs, list))
        debug._assert(branch is None or isinstance(branch,Branch))
        for input in inputs:
            debug._assert(input is not None)

        # Do type checking
        functionInputTypes = ca_function.inputTypes(function)
        variableArgs = ca_function.variableArgs(function)

        # Check if they have provided the correct number of arguments
        if not variableArgs:
            requiredArgCount = len(functionInputTypes)
            if len(inputs) != requiredArgCount:
                raise errors.WrongNumberOfArguments(ca_function.name(function),
                       requiredArgCount, len(inputs))

        def typeCheck(inputType, requiredType):
            # 'ref' type matches anything
            if requiredType is builtins.REFERENCE_TYPE:
                return True

            # 'any' type matches anything
            if requiredType is builtins.ANY_TYPE:
                return True

            # Temp: accept int for float
            if (inputType is builtins.INT_TYPE and 
                    requiredType is builtins.FLOAT_TYPE):
                return True

            return inputType is requiredType

        for index in range(len(inputs)):
            inputType = inputs[index].getType()
            if ca_function.variableArgs(function):
                requiredType = functionInputTypes[0]
            else:
                requiredType = functionInputTypes[index]
                
            if not typeCheck(inputType, requiredType):
                raise errors.WrongType(ca_function.name(function), 
                        requiredType.getIdentifier(), inputType.getIdentifier())

        # Try to reuse an existing term
        if not ca_function.hasState(function):
            existing = findExistingEquivalent(function, inputs)
            if existing:
                return existing

        newTerm = self._newTerm(branch)
        newTerm.functionTerm = function
        newTerm.inputs = list(inputs)

        outputType = ca_function.outputType(function)

        # Initialize cachedData
        debug._assert(outputType is not None)

        initializeFunc = ca_type.initialize(outputType)

        debug._assert(initializeFunc is not None)

        initializeFunc(newTerm)

        # Initialize the term, if this function has an initializeFunc
        if ca_function.initializeFunc(function):
            ca_function.initializeFunc(function)(newTerm.executionContext)

        # Immediately update, if this is a pure function, and all our inputs
        # are ready.
        if (ca_function.pureFunction(function) and
                all([term.outputReady for term in inputs])):
            newTerm.execute()

        # Add ourselves to .users of all our inputs and function
        function.users.add(newTerm)
        for input in inputs:
            input.users.add(newTerm)

        return newTerm

    def createConstant(self, type, branch=None):
        debug._assert(isinstance(type, Term))

        # Fetch the constant function for this type
        constantFunc = self.createTerm(builtins.CONST_GENERATOR, [type])

        # Create the term
        return self.createTerm(constantFunc, [], branch)

    def createVariable(self, type, branch=None):
        debug._assert(isinstance(type, Term))

        # Fetch the variable function for this type
        variableFunc = self.createTerm(builtins.VARIABLE_GENERATOR, [type])

        # Create the term
        return self.createTerm(variableFunc, [], branch)

    def deleteTerm(self, term):
        """
        Removes all external references to 'term', so that it will be garbage
        collected. This function does not remove 'term' from its branch, the
        caller should probably do that as well.
        """
        debug._assert(isinstance(term, Term))

        term.functionTerm.users.remove(term)
        for input in term.inputs:
            input.users.remove(term)

        term.deleted = True

    def bindName(self, term, name, allowOverwrite=False):
        return self.mainBranch.bindName(term,name,allowOverwrite)

    def containsName(self, name):
        "Returns True if this codeunit defines the given name"
        return self.mainBranch.containsName(name)

    def getNamed(self, name):
        debug._assert(isinstance(name,str))
        return self.mainBranch.getNamed(name)

    def getIdentifier(self, term):
        debug._assert(isinstance(term, Term))

        # Try to find this term in main namespace
        for (name,t) in self.mainBranch.iterateNamespace():
            if t is term:
                return name

        # Otherwise, use the default identifier
        return '$' + str(term.globalID)

    def getNames(self, term):
        """
        Returns a set of strings, for all the names that this term has.
        If the term is anonymous, returns an empty set.
        """
        debug._assert(isinstance(term, Term))
        result = set()
        for (name,t) in self.mainBranch.iterateNamespace():
            if t is term:
                result.add(name)
        return result

    def setInput(self, term, inputIndex, newInput):
        # Grow input array if necessary
        if newInput >= len(term.inputs):
            term.inputs.extend(itertools.repeat(None, inputIndex+1-len(term.inputs)))
        term.inputs[inputIndex] = newInput
        term.needsUpdate = True

    def execute(self):
        for term in self.mainBranch:
            term.execute()

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
