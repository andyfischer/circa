
def term(codeunit, term):
    return (codeunit.getIdentifier(term) + ': ' +
        codeunit.getIdentifier(term.functionTerm) +
        '(' + ','.join(map(codeunit.getIdentifier,term.inputs)) + ')'
