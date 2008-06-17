

class Branch(object):
    def __init__(self, ownerTerm):
        self._list = []
        self.ownerTerm = ownerTerm

    def append(self, term):
        # Make sure this term has the same codeUnit as our other terms
        if len(self._list) > 0:
            debug._assert(term.codeUnit is self._list[-1].codeUnit)
        self._list.append(term)

    def clear(self):
        if len(self._list) == 0:
            return

        codeUnit = self._list[0].codeUnit
        for term in self._list:
            codeUnit.deleteTerm(term)
        self._list = []

    def __iter__(self):
        for term in self._list:
            yield term
