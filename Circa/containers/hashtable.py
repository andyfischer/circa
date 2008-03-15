from Circa import bootstrap as bootstrap_module

from Circa import (
      ca_function
   )

class HashtableInsert(ca_function.Function):
   def pythonInit(self, term):
      hashtable = term.inputs[0]
      key = term.inputs[0]
      value = term.inputs[1]
      hashtable.pythonValue[key] = value

class HashtableLookup(ca_function.Function):
   def preemptTermCreate(self, inputs):
      hashtable = inputs[0]
      key = inputs[1]
      # todo: make sure hashtable is insert-only
      return hashtable.pythonValue[key]

def initTerm(term):
   term.pythonValue = {}

def handleConnect(term, key, value):
   term.pythonValue[key] = value

def handleDisconnect(term, key, value):
   del term.pythonValue[key]


def bootstrap():


Circa Functions we need..

hashInsert
hashLookup
future: hash get-all
future: hash iterate

hashtable can have "add-once-only" access, which allows for certain optimizations

hashLookup can, instead of creating a term, swap itself for the result, if the table
has "add-once-only" access

Changes to Function necessary:
   function that is called on term creation
   function that is able to preempt term creation


