// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "branch.h"
#include "function.h"
#include "runtime.h"
#include "term.h"
#include "type.h"

namespace circa {

void print_term_extended(Term* term, std::ostream &output)
{
    if (term == NULL) {
        output << "<NULL>";
        return;
    }

    std::string name = term->name;
    std::string funcName = term->function->name;
    std::string typeName = term->type->name;

    if (name != "")
        output << "'" << name << "' ";

    output << funcName << "() -> " << typeName;

    if (term->hasError())
        output << " *" << term->getErrorMessage() << "*";

    output << std::endl;
}

void print_branch_extended(Branch& branch, std::ostream &output)
{
    for (int i=0; i < branch.numTerms(); i++) {
        Term* term = branch[i];

        print_term_extended(term, output);
    }
}

void print_terms(ReferenceList const& list, std::ostream &output)
{
    for (unsigned int i=0; i < list.count(); i++) {
        print_term_extended(list[i], output);
    }
}

ReferenceList list_all_pointers(Term* term)
{
    ReferenceList result;

    struct AppendPointersToList : PointerVisitor
    {
        ReferenceList& list;
        AppendPointersToList(ReferenceList &_list) : list(_list) {}

        virtual void visitPointer(Term* term) {
            if (term != NULL)
                list.appendUnique(term);
        }
    };

    AppendPointersToList visitor(result);
    visit_pointers(term, visitor);

    return result;
}

bool is_using(Term* user, Term* usee)
{
    assert_good(user);
    assert_good(usee);
    if (user->inputs.contains(usee))
        return true;
    if (user->function == usee)
        return true;

    return false;
}

void check_pointers(Term* term)
{
    ReferenceList pointers = list_all_pointers(term);

    for (unsigned int i=0; i < pointers.count(); i++) {
        if (is_bad_pointer(pointers[i])) {
            // todo
        }
    }
}

bool function_allows_term_reuse(Function &function)
{
    if (function.stateType != VOID_TYPE)
        return false;

    if (!function.pureFunction)
        return false;

    return true;
}

bool is_equivalent(Term* target, Term* function, ReferenceList const& inputs)
{
    if (!function_allows_term_reuse(as_function(function)))
        return false;

    // Check inputs
    unsigned int numInputs = target->inputs.count();

    if (numInputs != inputs.count())
        return false;

    for (unsigned int i=0; i < numInputs; i++) {
        if (target->inputs[i] != inputs[i]) {
            return false;
        }
    }

    return true;
}

Term* find_equivalent(Branch &branch, Term* function, ReferenceList const& inputs)
{
    // This step is inefficient because we check every term in the branch.
    // Will improve in the future.
    for (int i=0; i < branch.numTerms(); i++) {
        Term *term = branch[i];
        if (is_equivalent(term, function, inputs))
            return term;
    }
    return NULL;
}

Term* find_equivalent(Term* function, ReferenceList const& inputs)
{
    if (!function_allows_term_reuse(as_function(function)))
        return false;

    Term* term = NULL;

    if (function->owningBranch != NULL)
        term = find_equivalent(*function->owningBranch, function, inputs);
    if (term != NULL)
        return term;

    // This step is inefficient because we check the owning branch of
    // every input, which may check some branches multiple times.
    for (unsigned int i=0; i < inputs.count(); i++) {
        if (inputs[i] == NULL)
            continue;

        Branch* branch = inputs[i]->owningBranch;
        if (branch != NULL)
            term = find_equivalent(*branch, function, inputs);
        if (term != NULL)
            return term;
    }

    return NULL;
}

} // namespace circa
