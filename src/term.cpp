// Copyright 2008 Andrew Fischer

#include <cstdio>
#include <sstream>
#include <iostream>

#include "bootstrapping.h"
#include "builtins.h"
#include "branch.h"
#include "errors.h"
#include "evaluation.h"
#include "function.h"
#include "operations.h"
#include "term.h"
#include "type.h"

namespace circa {

static int gNextGlobalID = 1;

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
    // Find all our users, and tell them to stop using us
    TermSet users = this->users;
    this->users = TermSet();

    TermMap nullPointerRemap;
    nullPointerRemap[this] = NULL;

    std::set<Term*>::const_iterator it;
    for (it = users._items.begin(); it != users._items.end(); ++it) {
        remap_pointers(*it, this, NULL);
    }

    if (this->owningBranch != NULL) {
        this->owningBranch->remapPointers(nullPointerRemap);
    }

    dealloc_value(this);
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

Term*
Term::field(std::string const& name) const
{
    int index = as_type(this->type)->getIndexForField(name);
    if (index == -1)
        return NULL;
    return this->fields[index];
}

bool
Term::equals(Term* term)
{
    if (this->type != term->type)
        return false;

    return as_type(this->type)->equals(this, term);
}

bool
Term::hasError() const
{
    return numErrors() != 0;
}

int
Term::numErrors() const
{
    return (int) this->errors.size();
}

std::string const&
Term::getError(int index) const
{
    return this->errors[index];
}

void
Term::clearErrors()
{
    this->errors.clear();
}

void
Term::pushError(std::string const& message)
{
    this->errors.push_back(message);
}

std::string
Term::getErrorMessage() const
{
    if (numErrors() == 0)
        return "";
    else
        return this->errors[numErrors()-1];
}

int& Term::asInt()
{
    return as_int(this);
}

float& Term::asFloat()
{
    return as_float(this);
}

std::string& Term::asString()
{
    return as_string(this);
}

bool& Term::asBool()
{
    return as_bool(this);
}

void Term::eval()
{
    evaluation::evaluate_term(this);
}

} // namespace circa
