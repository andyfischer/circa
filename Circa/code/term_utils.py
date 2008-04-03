
import pdb

from Circa import (
    ca_function
)

import term as term_module

def findExisting(functionTerm, inputs=[]):
   """
   This function finds an existing term that uses the given function, and has the given
   inputs. Returns None if none found.
   """
   if inputs is None:
      return None
 
   assert isinstance(functionTerm, term_module.Term)
   #assert isinstance(functionTerm.pythonValue, ca_function.Function)
 
   function = functionTerm.pythonValue
 
   # Try to find an existing term
   for input in inputs:
      assert input is not None
  
      for potentialMatch in input.users:
         # Check if they are using the same function
         if potentialMatch.functionTerm != functionTerm: continue
   
         # Check if all the inputs are the same
         def matches(pair):
            return pair[0].equals(pair[1])
   
         inputs_match = all(map(matches, zip(inputs, potentialMatch.inputs)))
   
         # Todo: allow for functions that don't care what the function order is
   
         if not inputs_match: continue
   
         # Looks like this term is the same as what they want
         return potentialMatch
 
   return None

def findExistingConstant(constantFunction, value):
   """
   This helper term attempts to find a constant with the given constant-func.
   """

   for possibleMatch in constantFunction.users:
       
      if possibleMatch.functionTerm != constantFunction:
         continue

      if possibleMatch.pythonValue == value:
         return possibleMatch

   return None

def findTrainingFunction(function):
   return function.pythonValue.trainingFunc
