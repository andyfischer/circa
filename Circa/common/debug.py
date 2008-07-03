
import pdb

from Circa.common import errors

def _assert(condition, msg=None):
    if not condition:
        if msg is not None:
            print "Assertion failed: " + msg
        pdb.set_trace() # An assertion has failed

def fail(message):
    print 'Debug.fail:', message
    pdb.set_trace()

def assertType(object, expectedType):
    if not isinstance(object, expectedType):
        print "Type fail, expected "+str(expectedType)+" but found "+str(type(object))
        pdb.set_trace()

def assertNullableType(object, expectedType):
    if not isinstance(object, expectedType) and object is not None:
        print "Type fail, expected "+str(expectedType)+" but found "+str(type(object))
        pdb.set_trace()
        
