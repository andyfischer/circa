// Copyright 2008 Paul Hodge

#include <cstdio>
#include <sstream>
#include <iostream>
#include <stdexcept>

#include "builtins.h"
#include "branch.h"
#include "errors.h"
#include "function.h"
#include "list.h"
#include "runtime.h"
#include "term.h"
#include "type.h"
#include "values.h"

#define CHECK_FOR_BAD_POINTERS true

namespace circa {

#if CHECK_FOR_BAD_POINTERS
static std::set<Term*> DEBUG_GOOD_POINTER_SET;
#endif

bool is_bad_pointer(Term* term)
{
    return DEBUG_GOOD_POINTER_SET.find(term) == DEBUG_GOOD_POINTER_SET.end();
}

void assert_good(Term* term)
{
#if CHECK_FOR_BAD_POINTERS
    if (is_bad_pointer(term))
        throw std::runtime_error("assert_good failed (bad term pointer)");
#endif
}

static int gNextGlobalID = 1;

Term::Term()
  : owningBranch(NULL),
    function(NULL),
    type(NULL),
    value(NULL),
    stealingOk(true),
    state(NULL),
    needsUpdate(true)
{
    globalID = gNextGlobalID++;

#if CHECK_FOR_BAD_POINTERS
    DEBUG_GOOD_POINTER_SET.insert(this);
#endif
}

Term::~Term()
{
    assert_good(this);

    ReferenceMap nullPointerRemap;
    nullPointerRemap[this] = NULL;

    dealloc_value(this);

    if (owningBranch != NULL) {
        owningBranch->termDeleted(this);
    }

#if CHECK_FOR_BAD_POINTERS
    DEBUG_GOOD_POINTER_SET.erase(this);
#endif
}

std::string
Term::toString()
{
    Type::ToStringFunc func = as_type(this->type).toString;

    if (func == NULL) {
        return std::string("<" + as_type(this->type).name + " " + findName() + ">");
    } else {
        return func(this);
    }
}

std::string
Term::findName()
{
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
Term::field(std::string const& name)
{
    return get_field(this, name);
}

bool
Term::equals(Term* term)
{
    if (this->type != term->type)
        return false;

    return as_type(this->type).equals(this, term);
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

Term*& Term::asRef()
{
    return as_ref(this);
}

List& Term::asList()
{
    return as_list(this);
}

void Term::eval()
{
    evaluate_term(this);
}

} // namespace circa
