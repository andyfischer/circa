
from Circa.common import debug

class Branch(object):
    def __init__(self, ownerTerm):
        self._list = []
        self._namespace = {}
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
        self._namespace = {}

    def containsName(self, name):
        return name in self._namespace

    def getNamed(self, name):
        return self._namespace[name]

    def bindName(self, term, name, allowOverwrite=False):
        """
        Bind the name to the given term. If allowOverwrite is false, an
        error will be thrown if this name is already bound.
        """
        debug._assert(isinstance(name, str))
        if not allowOverwrite and name in self._namespace:
            raise Exception("The name "+name+" is already bound. (to allow "
                + "this, you can use the allowOverwrite parameter)")

        self._namespace[name] = term

    def iterateNamespace(self):
        return self._namespace.items()

    def __iter__(self):
        for term in self._list:
            yield term
