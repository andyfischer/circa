// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "building.h"
#include "debug.h"
#include "hashtable.h"
#include "heap_debugging.h"
#include "kernel.h"
#include "inspection.h"
#include "names.h"
#include "string_type.h"
#include "term.h"
#include "type.h"
#include "world.h"

#include "term.h"

namespace circa {

Term::Term()
  : type(NULL),
    function(NULL),
    uniqueOrdinal(0),
    owningBlock(NULL),
    index(0),
    nestedContents(NULL)
{
    id = global_world()->nextTermID++;

    debug_register_valid_object(this, TERM_OBJECT);

    set_hashtable(&properties);

    if (id == DEBUG_BREAK_ON_TERM)
        ca_debugger_break();
}

Term::~Term()
{
    debug_unregister_valid_object(this, TERM_OBJECT);
}

Term::Input::Input() : term(NULL)
{
    set_hashtable(&properties);
}

Term::Input::Input(Term* t) : term(t)
{
    set_hashtable(&properties);
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
    else if (index == 1)
        return declared_type_term((Term*) this);
    else
        return input(index - 2);
}

int
Term::numDependencies() const
{
    return numInputs() + 2;
}

void
Term::setDependency(int index, Term* term)
{
    if (index == 0)
        change_function(this, term);
    else if (index == 1)
        change_declared_type(this, unbox_type(term));
    else
        set_input(this, index - 2, term);
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
    return term_get_property(this, name) != NULL;
}

void Term::removeProperty(const char* name)
{
    term_remove_property(this, name);
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
    if (term->type != NULL) {
        type_decref(term->type);
        term->type = NULL;
    }
    term->function = NULL;

    delete term;
}

caValue* term_insert_property(Term* term, const char* name)
{
    Value str;
    set_string(&str, name);
    return hashtable_insert(&term->properties, &str);
}

void term_set_property(Term* term, const char* name, caValue* value)
{
    swap(value, term_insert_property(term, name));
}

caValue* term_get_property(Term* term, const char* name)
{
    Value str;
    set_string(&str, name);
    return hashtable_get(&term->properties, &str);
}

void term_remove_property(Term* term, const char* name)
{
    Value str;
    set_string(&str, name);
    hashtable_remove(&term->properties, &str);
}

void term_move_property(Term* from, Term* to, const char* propName)
{
    caValue* existing = term_get_property(from, propName);
    if (existing == NULL)
        return;

    term_set_property(to, propName, existing);
    term_remove_property(from, propName);
}

caValue* term_get_input_property(Term* term, int inputIndex, const char* name)
{
    Term::Input* info = term->inputInfo(inputIndex);
    if (info == NULL)
        return NULL;
    return hashtable_get(&info->properties, name);
}
caValue* term_insert_input_property(Term* term, int inputIndex, const char* name)
{
    Value str;
    set_string(&str, name);
    return hashtable_insert(&term->inputInfo(inputIndex)->properties, &str);
}

bool term_get_bool_input_prop(Term* term, int inputIndex, const char* name, bool defaultValue)
{
    caValue* value = term_get_input_property(term, inputIndex, name);
    if (value == NULL)
        return defaultValue;
    return as_bool(value);
}

const char* term_get_string_input_prop(Term* term, int inputIndex, const char* name, const char* defaultValue)
{
    caValue* value = term_get_input_property(term, inputIndex, name);
    if (value == NULL)
        return defaultValue;
    return as_cstring(value);
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
Block* term_function(Term* term)
{
    return nested_contents(term->function);
}

bool is_type(Term* term)
{
    return is_value(term) && is_type(&term->value);
}
bool is_function(Term* term)
{
    if (term == NULL)
        return false;
    return term->function == FUNCS.function_decl;
}

Type* as_type(Term* term)
{
    return as_type(term_value(term));
}
int term_line_number(Term* term)
{
    return term->sourceLoc.line;
}

} // namespace circa
