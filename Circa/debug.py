
import pdb

def Assert(condition):
   if not condition:
      pdb.set_trace() # An assertion has failed
