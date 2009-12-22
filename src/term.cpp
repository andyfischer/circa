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
    index(0),
    flags(0),
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
    Term* existing = property(name);

    if (existing != NULL) {
        if (existing->type != type)
            throw std::runtime_error("Property "+name+" exists with different type");

        return existing;
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

bool
Term::hasError() const
{
    return flags & TERM_FLAG_ERRORED;
}

void
Term::setHasError(bool error)
{
    if (error)
        flags = flags | TERM_FLAG_ERRORED;
    else
        flags = flags & ~TERM_FLAG_ERRORED;
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

} // namespace circa
