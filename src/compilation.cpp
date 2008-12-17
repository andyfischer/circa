// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "branch.h"
#include "compilation.h"

namespace circa {

Term*
CompilationContext::findNamed(std::string const& name) const
{
    assert(!stack.empty());

    for (size_t i = stack.size() - 1; i >= 0; i--) {

        Term* term = stack[i].branch->findNamed(name);

        if (term != NULL)
            return term;
    }

    return NULL;
}

CompilationContext::Scope const&
CompilationContext::topScope() const
{
    return stack.back();
}

Branch&
CompilationContext::topBranch() const
{
    assert(!stack.empty());
    return *stack.back().branch;
}

void
CompilationContext::push(Branch* branch, Term* branchOwner)
{
    stack.push_back(Scope(branch, branchOwner));
}

void
CompilationContext::pop()
{
    stack.pop_back();
}

} // namespace circa
