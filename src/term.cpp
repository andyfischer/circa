// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "branch.h"
#include "building.h"
#include "builtins.h"
#include "debug_valid_objects.h"
#include "errors.h"
#include "term.h"
#include "type.h"

#include "term.h"

namespace circa {

static unsigned int gNextGlobalID = 1;

Term::Term()
  : owningBranch(NULL),
    index(0),
    registerIndex(-1),
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

TaggedValue* Term::property(std::string const& name)
{
    return properties.get(name.c_str());
}

bool Term::hasProperty(std::string const& name)
{
    return properties.contains(name.c_str());
}

TaggedValue* Term::addProperty(std::string const& name, Term* type)
{
    TaggedValue* prop = properties.insert(name.c_str());
    Type* valueType = type_contents(type);

    if (!is_null(prop) && prop->value_type != valueType)
        internal_error("Property "+name+" exists with different type");

    change_type(prop, valueType);
    return prop;
}

void Term::removeProperty(std::string const& name)
{
    properties.remove(name.c_str());
}

bool Term::boolProp(std::string const& name)
{
    TaggedValue* t = addProperty(name, BOOL_TYPE);
    return as_bool(t);
}
int Term::intProp(std::string const& name)
{
    TaggedValue* t = addProperty(name, INT_TYPE);
    return as_int(t);
}
float Term::floatProp(std::string const& name)
{
    TaggedValue* t = addProperty(name, FLOAT_TYPE);
    return as_float(t);
}
std::string const& Term::stringProp(std::string const& name)
{
    TaggedValue* t = addProperty(name, STRING_TYPE);
    return as_string(t);
}

void Term::setIntProp(std::string const& name, int i)
{
    TaggedValue* t = addProperty(name, INT_TYPE);
    set_int(t, i);
}

void Term::setFloatProp(std::string const& name, float f)
{
    TaggedValue* t = addProperty(name, FLOAT_TYPE);
    set_float(t, f);
}

void Term::setBoolProp(std::string const& name, bool b)
{
    TaggedValue* t = addProperty(name, BOOL_TYPE);
    set_bool(t, b);
}

void Term::setStringProp(std::string const& name, std::string const& s)
{
    TaggedValue* t = addProperty(name, STRING_TYPE);
    set_string(t, s);
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

void assert_term_invariants(Term* t)
{
    // Make sure the value type matches the declared type.
    if (t->type == INT_TYPE)
        ca_assert(is_int(t));

    if (t->type != INT_TYPE)
        ca_assert(!is_int(t));
}

} // namespace circa
