// Copyright 2008 Paul Hodge

#include <cstdio>
#include <sstream>
#include <iostream>
#include <stdexcept>

#include "builtins.h"
#include "builtin_types.h"
#include "branch.h"
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

void assert_good_pointer(Term* term)
{
#if CHECK_FOR_BAD_POINTERS
    if (is_bad_pointer(term))
        throw std::runtime_error("assert_good_pointer failed (bad term pointer)");
#endif
}

static unsigned int gNextGlobalID = 1;

Term::Term()
  : value(NULL),
    type(NULL),
    function(NULL),
    state(NULL),
    owningBranch(NULL),
    stealingOk(true),
    needsUpdate(true),
    myBranch(NULL)
{
    globalID = gNextGlobalID++;

#if CHECK_FOR_BAD_POINTERS
    DEBUG_GOOD_POINTER_SET.insert(this);
#endif
}

Term::~Term()
{
    if (owningBranch != NULL)
        assert(DEBUG_CURRENTLY_INSIDE_BRANCH_DESTRUCTOR > 0);

    assert_good_pointer(this);

    ReferenceMap nullPointerRemap;
    nullPointerRemap[this] = NULL;

    dealloc_value(this);

    delete myBranch;
    myBranch = NULL;

#if CHECK_FOR_BAD_POINTERS
    DEBUG_GOOD_POINTER_SET.erase(this);
#endif
}

Term*
Term::input(int index) const
{
    return this->inputs[index];
}

std::string
Term::toString()
{
    Type::ToStringFunc func = as_type(this->type).toString;

    if (func == NULL) {
        return std::string("<" + as_type(this->type).name + " " + name + ">");
    } else {
        return func(this);
    }
}

Term*
Term::property(std::string const& name)
{
    return getMyBranch()->getNamed(name);
}

Term*
Term::addProperty(std::string const& name, Term* type)
{
    Term* term = create_value(getMyBranch(), type);
    assert(term != NULL);
    getMyBranch()->bindName(term, name);
    return term;
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
    return this->errors.size() != 0;
}

void
Term::clearError()
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
    if (!hasError())
        return "";
    else
        return this->errors[0];
}

Branch*
Term::getMyBranch()
{
    if (myBranch == NULL) {
        myBranch = new Branch();
        myBranch->owningTerm = this;
    }

    return myBranch;
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

Term* Term::field(int index)
{
    return get_field(this, index);
}

Term* Term::field(std::string const& fieldName)
{
    return get_field(this, fieldName);
}

} // namespace circa
