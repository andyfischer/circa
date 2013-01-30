// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "building.h"
#include "kernel.h"
#include "debug.h"
#include "heap_debugging.h"
#include "inspection.h"
#include "names.h"
#include "string_type.h"
#include "term.h"
#include "type.h"
#include "world.h"

#include "term.h"

namespace circa {

Term::Term()
  : weakPtr(0),
    type(NULL),
    function(NULL),
    uniqueOrdinal(0),
    owningBlock(NULL),
    index(0),
    nestedContents(NULL)
{
    id = global_world()->nextTermID++;

    debug_register_valid_object(this, TERM_OBJECT);

    if (id == DEBUG_BREAK_ON_TERM)
        ca_debugger_break();
}

Term::~Term()
{
    debug_unregister_valid_object(this, TERM_OBJECT);
    weak_ptr_set_null(weakPtr);
}

const char*
Term::nameStr()
{
    return as_cstring(&nameValue);
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
    return (int) this->inputs.size();
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

Block*
Term::contents()
{
    return nested_contents(this);
}
Term*
Term::contents(int index)
{
    return nested_contents(this)->get(index);
}
Term*
Term::contents(const char* name)
{
    return nested_contents(this)->get(name);
}

std::string
Term::toString()
{
    return to_string(term_value(this));
}

bool Term::hasProperty(const char* name)
{
    return properties.contains(name);
}

void Term::removeProperty(const char* name)
{
    properties.remove(name);
}

caValue* Term::getProp(const char* name)
{
    return term_get_property(this, name);
}

bool Term::boolProp(const char* name, bool defaultValue)
{
    caValue* value = term_get_property(this, name);
    if (value == NULL)
        return defaultValue;
    else
        return as_bool(value);
}

float Term::floatProp(const char* name, float defaultValue)
{
    caValue* value = term_get_property(this, name);
    if (value == NULL)
        return defaultValue;
    else
        return as_float(value);
}

int Term::intProp(const char* name, int defaultValue)
{
    caValue* value = term_get_property(this, name);
    if (value == NULL)
        return defaultValue;
    else
        return as_int(value);
}
std::string Term::stringProp(const char* name, const char* defaultValue)
{
    caValue* value = term_get_property(this, name);
    if (value == NULL)
        return defaultValue;
    else
        return as_string(value);
}

void Term::setProp(const char* name, caValue* value)
{
    caValue* t = term_insert_property(this, name);
    copy(value, t);
}

void Term::setIntProp(const char* name, int i)
{
    caValue* t = term_insert_property(this, name);
    set_int(t, i);
}

void Term::setFloatProp(const char* name, float f)
{
    caValue* t = term_insert_property(this, name);
    set_float(t, f);
}

void Term::setBoolProp(const char* name, bool b)
{
    caValue* t = term_insert_property(this, name);
    set_bool(t, b);
}

void Term::setStringProp(const char* name, std::string const& s)
{
    caValue* t = term_insert_property(this, name);
    set_string(t, s);
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
    term->inputs.clear();
    term->type = NULL;
    term->function = NULL;

    delete term;
}

caValue* term_insert_property(Term* term, const char* name)
{
    return term->properties.insert(name);
}

void term_set_property(Term* term, const char* name, caValue* value)
{
    swap(value, term_insert_property(term, name));
}

caValue* term_get_property(Term* term, const char* name)
{
    INCREMENT_STAT(TermPropAccess);
    return term->properties[name];
}

void term_remove_property(Term* term, const char* name)
{
    term->properties.remove(name);
}

void term_move_property(Term* from, Term* to, const char* propName)
{
    if (!from->hasProperty(propName))
        return;

    term_set_property(to, propName, term_get_property(from, propName));
    term_remove_property(from, propName);
}

caValue* term_get_input_property(Term* term, int inputIndex, const char* name)
{
    Term::Input* info = term->inputInfo(inputIndex);
    if (info == NULL)
        return NULL;
    return dict_get(&info->properties, name);
}
caValue* term_insert_input_property(Term* term, int inputIndex, const char* name)
{
    return dict_insert(&term->inputInfo(inputIndex)->properties, name);
}

int term_get_int_prop(Term* term, Symbol prop, int defaultValue)
{
    return term->intProp(builtin_symbol_to_string(prop), defaultValue);
}
const char* term_get_string_prop(Term* term, Symbol prop, const char* defaultValue)
{
    return term->stringProp(builtin_symbol_to_string(prop), defaultValue).c_str();
}
void term_set_int_prop(Term* term, Symbol prop, int value)
{
    term->setIntProp(builtin_symbol_to_string(prop), value);
}
void term_set_string_prop(Term* term, Symbol prop, const char* value)
{
    term->setStringProp(builtin_symbol_to_string(prop), value);
}
bool is_input_implicit(Term* term, int index)
{
    caValue* val = term_get_input_property(term, index, "implicit");
    if (val == NULL)
        return false;
    return as_bool(val);
}
void set_input_implicit(Term* term, int index, bool implicit)
{
    set_bool(term_insert_input_property(term, index, "implicit"), true);
}
bool is_input_hidden(Term* term, int index)
{
    caValue* val = term_get_input_property(term, index, "hidden");
    if (val == NULL)
        return false;
    return as_bool(val);
}
void set_input_hidden(Term* term, int index, bool hidden)
{
    set_bool(term_insert_input_property(term, index, "hidden"), true);
}

caValue* term_name(Term* term)
{
    return &term->nameValue;
}

caValue* term_value(Term* term)
{
    return &term->value;
}

bool is_type(Term* term)
{
    return is_value(term) && is_type(&term->value);
}
bool is_function(Term* term)
{
    if (term == NULL)
        return false;
    return is_value(term) && is_function(&term->value);
}
Function* as_function(Term* term)
{
    return as_function(term_value(term));
}
Type* as_type(Term* term)
{
    return as_type(term_value(term));
}

} // namespace circa
