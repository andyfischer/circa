
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

Term::~Term()
{
    dealloc_value(this);
}

Type*
Term::getType() const
{
    return as_type(this->type);
}

std::string
Term::toString()
{
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
    if (this == NULL)
        return "<NULL TERM>";

    Branch* branch = this->owningBranch;

    if (branch == NULL)
        return "";

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

void initialize_term(Branch* kernel)
{
}

} // namespace circa
