// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

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
    hasError(false),
    refCount(0)
{
    globalID = gNextGlobalID++;

    METRIC_TERMS_CREATED++;

    register_good_pointer(this);
}

Term::~Term()
{
    METRIC_TERMS_DESTROYED++;
}

Term*
Term::input(int index) const
{
    return this->inputs[index];
}

int
Term::numInputs() const
{
    return this->inputs.length();
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

    return create_value(properties, type, name);
}

void Term::removeProperty(std::string const& name)
{
    properties.remove(name);
}

bool& Term::boolProp(std::string const& name)
{
    Term* t = addProperty(name, BOOL_TYPE);
    return as_bool(t);
}
int& Term::intProp(std::string const& name)
{
    Term* t = addProperty(name, INT_TYPE);
    return as_int(t);
}
float& Term::floatProp(std::string const& name)
{
    Term* t = addProperty(name, FLOAT_TYPE);
    return as_float(t);
}
std::string& Term::stringProp(std::string const& name)
{
    Term* t = addProperty(name, STRING_TYPE);
    return as_string(t);
}
Ref& Term::refProp(std::string const& name)
{
    Term* t = addProperty(name, REF_TYPE);
    return as_ref(t);
}

bool Term::boolPropOptional(std::string const& name, bool defaultValue)
{
    if (hasProperty(name)) return boolProp(name);
    else return defaultValue;
}

float Term::floatPropOptional(std::string const& name, float defaultValue)
{
    if (hasProperty(name)) return floatProp(name);
    else return defaultValue;
}

int Term::intPropOptional(std::string const& name, int defaultValue)
{
    if (hasProperty(name)) return intProp(name);
    else return defaultValue;
}
std::string Term::stringPropOptional(std::string const& name, std::string const& defaultValue)
{
    if (hasProperty(name)) return stringProp(name);
    else return defaultValue;
}

void Term::attachErrorMessage(std::string const& message)
{
    hasError = true;
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

float Term::toFloat()
{
    return to_float(this);
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
    return as_ref(this);
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
