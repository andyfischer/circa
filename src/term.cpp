// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <cstdio>
#include <sstream>
#include <iostream>
#include <stdexcept>

#include "circa.h"
#include "debug_valid_objects.h"

namespace circa {

static unsigned int gNextGlobalID = 1;

Term::Term()
  : owningBranch(NULL),
    index(0),
    flags(0),
    refCount(0)
{
    globalID = gNextGlobalID++;

    METRIC_TERMS_CREATED++;
    debug_register_valid_object(this, TERM_OBJECT);
    nestedContents.owningTerm = this;
}

Term::~Term()
{
    METRIC_TERMS_DESTROYED++;
    debug_unregister_valid_object(this);
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

bool Term::boolProp(std::string const& name)
{
    Term* t = addProperty(name, BOOL_TYPE);
    return as_bool(t);
}
int Term::intProp(std::string const& name)
{
    Term* t = addProperty(name, INT_TYPE);
    return as_int(t);
}
float Term::floatProp(std::string const& name)
{
    Term* t = addProperty(name, FLOAT_TYPE);
    return as_float(t);
}
std::string const& Term::stringProp(std::string const& name)
{
    Term* t = addProperty(name, STRING_TYPE);
    return as_string(t);
}
Ref Term::refProp(std::string const& name)
{
    Term* t = addProperty(name, REF_TYPE);
    return as_ref(t);
}

void Term::setIntProp(std::string const& name, int i)
{
    Term* t = addProperty(name, INT_TYPE);
    set_int(t, i);
}

void Term::setFloatProp(std::string const& name, float f)
{
    Term* t = addProperty(name, FLOAT_TYPE);
    set_float(t, f);
}

void Term::setBoolProp(std::string const& name, bool b)
{
    Term* t = addProperty(name, BOOL_TYPE);
    set_bool(t, b);
}

void Term::setStringProp(std::string const& name, std::string const& s)
{
    Term* t = addProperty(name, STRING_TYPE);
    set_str(t, s);
}

void Term::setRefProp(std::string const& name, Term* r)
{
    Term* t = addProperty(name, REF_TYPE);
    set_ref(t, r);
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

Term* alloc_term()
{
    // This function is not very useful now, but we may switch to using a memory
    // pool in the future.
    Term* term = new Term();
    return term;
}

void ca_assert_term_invariants(Term* t)
{
    // Make sure the value type matches the declared type.
    if (t->type == INT_TYPE)
        ca_assert(is_int(t));

    if (t->type != INT_TYPE)
        ca_assert(!is_int(t));
}

} // namespace circa
