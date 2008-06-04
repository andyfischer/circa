#
# All pieces of data in Circa are wrapped in Terms.
#
# This class should be as minimal as possible.
# 
# When possible, code for manimulating or comparing terms should be placed elsewhere,
# such as in CodeUnit

import ca_function, ca_type

nextGlobalID = 1

class Term(object):
    def __init__(self):
        "Use 'createTerm' instead of calling this constructor"
   
        self.inputs = []
        self.functionTerm = None
        self.users = set()

        # The output value of this term
        self.cachedValue = None

        # True if 'cachedValue' is out of date
        self.needsUpdate = True

        self.state = None
        self.codeUnit = None
        self.branch = None
        self.givenName = None
        self.debugName = None
        self.feedbackAccumulator = None
   
        global nextGlobalID
        self.globalID = nextGlobalID
        nextGlobalID += 1

    def getInput(self, inputIndex):
        "Return an input term"
        return self.inputs[inputIndex]
  
    def getType(self):
        "Returns this term's output type"
        return ca_function.outputType(self.functionTerm)
  
    def update(self):
        if not self.needsUpdate:
            return

        # Functions with side effects should not run during an
        # update() call.
        if not ca_function.pureFunction(self.functionTerm):
            return

        ca_function.evaluateFunc(self.functionTerm)(self)

        self.needsUpdate = False

    def execute(self):
        if not self.needsUpdate:
            return
        ca_function.evaluateFunc(self.functionTerm)(self)
        self.needsUpdate = False
  
    # value accessors
    def __int__(self):
        try: return int(self.pythonValue)
        except: return 0
  
    def __float__(self):
        try: return float(self.pythonValue)
        except: return 0.0
  
    def __str__(self):
        return ca_type.toStringFunc(self.getType())(self)
