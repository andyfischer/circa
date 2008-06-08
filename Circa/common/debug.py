
import pdb

from Circa.common import errors

def _assert(condition):
   if not condition:
      pdb.set_trace() # An assertion has failed

def fail(message):
    print 'Debug.fail:', message
    pdb.set_trace()

def assertType(object, expectedType, variableName):
    if not isinstance(object, expectedType):
        raise errors.TypeCheckFail(variableName, expectedType)
