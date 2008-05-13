"""
This module contains sanity-checking functions for CodeUnit, for testing purposes
"""

class SanityCheckFailed(Exception):
   pass

def assertf(condition, message):
   if not condition:
      raise SanityCheckFailed(message)

def check(code_unit):
 
   alreadyEncountered = set()
 
   for term in code_unit.iterateTerms():
 
      assertf(term.codeUnit == code_unit, "term.codeUnit points to something else")
  
      assertf(term not in alreadyEncountered, "A term occurs more than once")
      alreadyEncountered.add(term)
  
      for user in term.users:
         assertf(term in user.inputs, "term.users contained a term that wasn't using it")
  
      for input in term.inputs:
         assertf(term in input.users, "One of term's inputs did not have term as an input")
  
      return True
