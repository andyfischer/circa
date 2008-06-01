
import pdb

def _assert(condition):
   if not condition:
      pdb.set_trace() # An assertion has failed

def fail(message):
    print 'Debug.fail:', message
    pdb.set_trace()

