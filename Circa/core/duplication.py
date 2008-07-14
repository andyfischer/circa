

def duplicate_branch(source_branch, into_branch):
    term_remaps = {}
    intoCodeUnit = into_branch.owningCodeUnit()

    def getPossibleRemap(term):
        if term in term_remaps:
            return term_remaps[term]
        else:
            return term

    for term in source_branch:
        remapped_inputs = map(getPossibleRemap, term_reterm.inputs)
        intoCodeUnit.createTerm(term.functionTerm, remapped_inputs)
