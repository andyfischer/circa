// Copyright 2008 Paul Hodge

#include <cstdio>
#include <sstream>
#include <iostream>
#include <stdexcept>

#include "circa.h"

namespace circa {

static unsigned int gNextGlobalID = 1;

Term::Term()
  : value(NULL),
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
    // Clear references
    std::vector<Ref*>::iterator it;
    for (it = refs.begin(); it != refs.end(); ++it) {
        (*it)->_target = NULL;
    }
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

Term* Term::property(std::string const& name) const
{
    return properties[name];
}

bool Term::hasProperty(std::string const& name) const
{
    return properties.contains(name);
}

Term* Term::addProperty(std::string const& name, Term* type)
{
    if (hasProperty(name)) {
        if (property(name)->type != type)
            throw std::runtime_error("Property "+name+" exists with different type");

        return property(name);
    }

    return properties.addSlot(name, type);
}

void Term::removeProperty(std::string const& name)
{
    properties.remove(name);
}

bool Term::hasError() const
{
    return hasProperty("error");
}

void Term::clearError()
{
    if (hasProperty("error"))
        removeProperty("error");
}

void Term::pushError(std::string const& message)
{
    addProperty("error", STRING_TYPE);
    as_string(property("error")) = message;
}

std::string Term::getErrorMessage() const
{
    if (hasProperty("error"))
        return as_string(property("error"));
    else
        return "";
}

bool Term::isStateful() const
{
    if (hasProperty("stateful"))
        return as_bool(property("stateful"));
    else
        return false;
}

void Term::setIsStateful(bool value)
{
    addProperty("stateful", BOOL_TYPE);
    as_bool(property("stateful")) = value;
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

Branch& Term::asBranch()
{
    return as_branch(this);
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
