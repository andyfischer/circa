
class _SpyFunction(object):
    def __init__(self, spyObject, name):
        self.name = name
        self.spyObject = spyObject

    def __call__(self, args):
        spyObject._notifyOfCall(digestFunctionCall(name, args))

def digestFunctionCall(name, args):
    """
    Take a function call and store it in some common format.
    (currently stored as a string)
    """
    return str(name) + ' '.join(map(str,args))


class SpyObject(object):
    def __init__(self):
        self.expectedCalls = []
        self.currentExpectedCallIndex = 0

    def __getattr__(self, name):
        return _SpyFunction(self, name)

    def _notifyOfCall(self, functionCall):
        if self.currentExpectedCallIndex >= len(self.expectedCalls):
            raise Exception('Got call "%s" but didn\'t expect any more calls'
                    % functionCall)

        expectedCall = self.expectedCalls[self.currentExpectedCallIndex]
        if functionCall != expectedCall:
            raise Exception('Function call "%s" didn\'t match expected "%s"'
                    % (functionCall, expectedCall)

        self.currentExpectedCallIndex += 1

    def expectCall(self, name, args):
        self.currentExpectedCallIndex.append(digestFunctionCall(name,args))
