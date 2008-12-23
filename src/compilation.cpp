// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "compilation.h"
#include "ref_list.h"
#include "runtime.h"
#include "term.h"

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

Term* find_and_apply_function(CompilationContext &context,
        std::string const& functionName,
        ReferenceList inputs)
{
    Term* function = context.findNamed(functionName);

    if (function == NULL) {
        std::cout << "warning: function not found: " << functionName << std::endl;

        Term* result = apply_function(context.topBranch(), UNKNOWN_FUNCTION, inputs);
        as_string(result->state) = functionName;
        return result;
    }

    return apply_function(context.topBranch(), function, inputs);
}


} // namespace circa
