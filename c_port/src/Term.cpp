
#include <cstdio>
#include <sstream>
#include <iostream>

#include "bootstrapping.h"
#include "builtins.h"
#include "branch.h"
#include "errors.h"
#include "function.h"
#include "operations.h"
#include "term.h"
#include "type.h"

namespace circa {

int gNextGlobalID = 1;

Term::Term()
  : owningBranch(NULL),
    function(NULL),
    value(NULL),
    type(NULL),
    state(NULL),
    needsUpdate(true)
{
    globalID = gNextGlobalID++;
}

Type*
Term::getType() const
{
    return as_type(this->type);
}

std::string
Term::toString()
{
    /*
    Term* result = apply_function(this->owningBranch,
            get_global("to-string"), TermList(this));
    execute(result);
    return as_string(result);
    */
    Type::ToStringFunc func = as_type(this->type)->toString;

    if (func == NULL) {
        return std::string("<" + as_type(this->type)->name + " " + findName() + ">");
    } else {
        return func(this);
    }
}

std::string
Term::findName()
{
    Branch* branch = this->owningBranch;

    if (branch == NULL)
        return "<null branch>";

    std::string name = branch->names.findName(this);

    if (name == "") {
        std::stringstream output;
        output << "#" << this->globalID;
        return output.str();
    } else {
        return name;
    }
}

int
Term::numErrors() const
{
    return (int) errors.size();
}

std::string const&
Term::getError(int index)
{
    return errors[index];
}

// TermList type
TermList* as_list(Term* term)
{
    if (term->type != LIST_TYPE)
        throw errors::TypeError(term, LIST_TYPE);
    return (TermList*) term->value;
}

void TermList_alloc(Term* caller)
{
    caller->value = new TermList();
}

void TermList_dealloc(Term* caller)
{
    delete as_list(caller);
}

void initialize_term(Branch* kernel)
{
    LIST_TYPE = quick_create_type(kernel, "List",
            TermList_alloc,
            TermList_dealloc,
            NULL);
}

} // namespace circa
