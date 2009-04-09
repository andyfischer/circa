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
    stealingOk(true),
    needsUpdate(true),
    refCount(0)
{
    globalID = gNextGlobalID++;

    register_good_pointer(this);
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
    return to_string(this);
}

bool
Term::hasValue()
{
    return value != NULL;
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

    return create_value(&properties, type, name);
}

void Term::removeProperty(std::string const& name)
{
    properties.remove(name);
}

bool& Term::boolProperty(std::string const& name)
{
    Term* t = addProperty(name, BOOL_TYPE);
    return as_bool(t);
}
int& Term::intProperty(std::string const& name)
{
    Term* t = addProperty(name, INT_TYPE);
    return as_int(t);
}
float& Term::floatProperty(std::string const& name)
{
    Term* t = addProperty(name, FLOAT_TYPE);
    return as_float(t);
}
std::string& Term::stringProperty(std::string const& name)
{
    Term* t = addProperty(name, STRING_TYPE);
    return as_string(t);
}

bool Term::boolPropertyOptional(std::string const& name, bool defaultValue)
{
    if (hasProperty(name)) return boolProperty(name);
    else return defaultValue;
}

float Term::floatPropertyOptional(std::string const& name, float defaultValue)
{
    if (hasProperty(name)) return floatProperty(name);
    else return defaultValue;
}

int Term::intPropertyOptional(std::string const& name, int defaultValue)
{
    if (hasProperty(name)) return intProperty(name);
    else return defaultValue;
}
std::string Term::stringPropertyOptional(std::string const& name, std::string const& defaultValue)
{
    if (hasProperty(name)) return stringProperty(name);
    else return defaultValue;
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

Ref& Term::asRef()
{
    return deref(this);
}

Branch& Term::asBranch()
{
    return as_branch(this);
}

Term* Term::field(int index)
{
    return as_branch(this)[index];
}

Term* Term::field(std::string const& fieldName)
{
    return as_branch(this)[fieldName];
}

} // namespace circa
