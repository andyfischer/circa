import pdb

def setValue(term, value):
    term.state = value
    term.cachedValue = value
    term.invalidate()
