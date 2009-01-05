// Copyright 2008 Paul Hodge

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
    assert(!scopeStack.empty());

    for (size_t i = scopeStack.size() - 1; i >= 0; i--) {

        Term* term = scopeStack[i].branch->findNamed(name);

        if (term != NULL)
            return term;
    }

    return NULL;
}

CompilationContext::Scope const&
CompilationContext::topScope() const
{
    return scopeStack.back();
}

Branch&
CompilationContext::topBranch() const
{
    assert(!scopeStack.empty());
    return *scopeStack.back().branch;
}

void
CompilationContext::pushScope(Branch* branch, Term* branchOwner)
{
    scopeStack.push_back(Scope(branch, branchOwner));
}

void
CompilationContext::pushExpressionFrame(bool insideExpression)
{
    // todo
}

void
CompilationContext::popExpressionFrame()
{
    // todo
}

void
CompilationContext::popScope()
{
    scopeStack.pop_back();
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

Term* create_comment(Branch& branch, std::string const& text)
{
    Term* result = apply_function(branch, COMMENT_FUNC, ReferenceList());
    as_string(result->state->field(0)) = text;
    return result;
}



} // namespace circa
