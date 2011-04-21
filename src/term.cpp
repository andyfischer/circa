// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "building.h"
#include "builtins.h"
#include "debug_valid_objects.h"
#include "errors.h"
#include "introspection.h"
#include "refactoring.h"
#include "term.h"
#include "type.h"

#include "term.h"

namespace circa {

static unsigned int gNextGlobalID = 1;

Term::Term()
  : function(NULL),
    evaluateFunc(empty_evaluate_function),
    owningBranch(NULL),
    index(0),
    localsIndex(0),
    refCount(0)
{
    globalID = gNextGlobalID++;

    debug_register_valid_object(this, TERM_OBJECT);
    nestedContents.owningTerm = this;
}

Term::~Term()
{
    debug_unregister_valid_object(this);
}

Term*
Term::input(int index) const
{
    if (index >= numInputs())
        return NULL;
    return this->inputs[index].term;
}

Term::Input*
Term::inputInfo(int index)
{
    if (index >= numInputs())
        return NULL;
    return &this->inputs[index];
}

int
Term::numInputs() const
{
    return this->inputs.size();
}

void
Term::inputsToList(RefList* out) const
{
    out->resize(numInputs());
    for (int i=0; i < numInputs(); i++)
        out->setAt(i, input(i));
}
void
Term::inputsToList(TermList& out) const
{
    out.resize(numInputs());
    for (int i=0; i < numInputs(); i++)
        out.setAt(i, input(i));
}

Term*
Term::dependency(int index) const
{
    if (index == 0)
        return this->function;
    else
        return input(index - 1);
}

int
Term::numDependencies() const
{
    return numInputs() + 1;
}

void
Term::setDependency(int index, Term* term)
{
    if (index == 0)
        change_function(this, term);
    else
        set_input(this, index - 1, term);
}

const char*
Term::getName(int index) const
{
    if (index == 0)
        return name.c_str();

    index -= 1;

    if (index >= additionalOutputNames.count())
        return "";

    return additionalOutputNames[index];
}

int
Term::nameCount() const
{
    return 1 + additionalOutputNames.count();
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
    Type* valueType = unbox_type(type);

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
    ca_assert(!(name == "syntax:postWhitespace" && s == "       "));
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

void dealloc_term(Term* term)
{
    delete term;
}

static void append_term_invariant_error(List* errors, Term* term,
        std::string const& msg)
{
    List& error = *List::cast(errors->append(), 1);
    set_string(error[0], msg);
}

void term_check_invariants(List* errors, Term* term)
{
    if (term->value_type == NULL)
        append_term_invariant_error(errors, term, "TaggedValue has null type");

    if (term->type != NULL) {

        bool typeOk = (term->type == ANY_TYPE)
            || (term->type == VOID_TYPE && is_null(term))
            || cast_possible(term, unbox_type(term->type));

        if (!typeOk) {
            std::string msg;
            msg += "Value has wrong type: term->type is " + term->type->name
                + ", tag is " + term->value_type->name;
            append_term_invariant_error(errors, term, msg);
        }
    }

    if (term->nestedContents.owningTerm != term)
        append_term_invariant_error(errors, term,
                "Term.nestedContents has wrong owningTerm");

    if (term->owningBranch != NULL) {
        Branch& branch = *term->owningBranch;
        if ((term->index >= branch.length())
                || (branch[term->index] != term)) {
            append_term_invariant_error(errors, term,
                    "Term.index doesn't resolve to this term in owningBranch");
        }
    }
}

} // namespace circa
