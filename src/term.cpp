// Copyright 2008 Andrew Fischer

#include <cstdio>
#include <sstream>
#include <iostream>
#include <stdexcept>

#include "builtins.h"
#include "builtin_types.h"
#include "branch.h"
#include "compound_value.h"
#include "debug.h"
#include "function.h"
#include "list.h"
#include "runtime.h"
#include "term.h"
#include "type.h"
#include "values.h"

namespace circa {

static unsigned int gNextGlobalID = 1;

Term::Term()
  : value(NULL),
    type(NULL),
    function(NULL),
    state(NULL),
    owningBranch(NULL),
    ownsValue(true),
    stealingOk(true),
    needsUpdate(true)
{
    globalID = gNextGlobalID++;

#if DEBUG_CHECK_FOR_BAD_POINTERS
    DEBUG_GOOD_POINTER_SET.insert(this);
#endif
}

Term::~Term()
{
}

Term*
Term::input(int index) const
{
    return this->inputs[index];
}

int
Term::numInputs() const
{
    return this->inputs.count();
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
    return properties[name];
}

bool Term::hasProperty(std::string const& name)
{
    return properties.contains(name);
}

Term*
Term::addProperty(std::string const& name, Term* type)
{
    return properties.addSlot(name, type);
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
