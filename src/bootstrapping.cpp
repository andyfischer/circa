
#include "common_headers.h"

#include "bootstrapping.h"
#include "branch.h"
#include "builtins.h"
#include "tokenizer.h"
#include "token_stream.h"
#include "operations.h"
#include "subroutine.h"

namespace token = circa::tokenizer;

namespace circa {

Term* quick_create_type(
        Branch* branch,
        std::string name,
        Type::AllocFunc allocFunc,
        Type::DeallocFunc deallocFunc,
        Type::DuplicateFunc duplicateFunc,
        Type::ToStringFunc toStringFunc)
{
    Term* typeTerm = create_constant(branch, TYPE_TYPE);
    as_type(typeTerm)->name = name;
    as_type(typeTerm)->alloc = allocFunc;
    as_type(typeTerm)->dealloc = deallocFunc;
    as_type(typeTerm)->duplicate = duplicateFunc;
    as_type(typeTerm)->toString = toStringFunc;
    branch->bindName(typeTerm, name);

    return typeTerm;
}

Term* quick_create_function(Branch* branch, string name, Function::EvaluateFunc evaluateFunc,
        TermList inputTypes, Term* outputType)
{
    Term* term = create_constant(branch, FUNCTION_TYPE);
    Function* func = as_function(term);
    func->name = name;
    func->evaluate = evaluateFunc;
    func->inputTypes = inputTypes;
    func->outputType = outputType;
    branch->bindName(term, name);
	return term;
}

void hosted_apply_function(Term* caller)
{
    recycle_value(caller->inputs[0], caller);
    Branch* branch = as_branch(caller);
    Term* function = caller->inputs[1];
    TermList* inputs = as_list(caller->inputs[2]);

    apply_function(branch, function, *inputs);
}

void hosted_to_string(Term* caller)
{
    as_string(caller) = caller->inputs[0]->toString();
}

void initialize_bootstrapped_code(Branch* kernel)
{
    Term* apply_function_f = quick_create_function(kernel, "apply-function",
        hosted_apply_function,
        TermList(BRANCH_TYPE, FUNCTION_TYPE, LIST_TYPE),
        BRANCH_TYPE);
    as_function(apply_function_f)->recycleInput = 0;

    quick_create_function(kernel, "to-string", hosted_to_string,
        TermList(ANY_TYPE), STRING_TYPE);

    // Parser
}

} // namespace circa
