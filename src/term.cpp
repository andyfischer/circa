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
        set_declared_type(this, unbox_type(term));
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

bool Term::hasProperty(Symbol key)
{
    return term_get_property(this, key) != NULL;
}

void Term::removeProperty(Symbol key)
{
    term_remove_property(this, key);
}

caValue* Term::getProp(Symbol key)
{
    return term_get_property(this, key);
}

bool Term::boolProp(Symbol key, bool defaultValue)
{
    caValue* value = term_get_property(this, key);
    if (value == NULL)
        return defaultValue;
    else
        return as_bool(value);
}

float Term::floatProp(Symbol key, float defaultValue)
{
    caValue* value = term_get_property(this, key);
    if (value == NULL)
        return defaultValue;
    else
        return as_float(value);
}

int Term::intProp(Symbol key, int defaultValue)
{
    caValue* value = term_get_property(this, key);
    if (value == NULL)
        return defaultValue;
    else
        return as_int(value);
}
std::string Term::stringProp(Symbol key, const char* defaultValue)
{
    caValue* value = term_get_property(this, key);
    if (value == NULL)
        return defaultValue;
    else
        return as_string(value);
}

void Term::setProp(Symbol key, caValue* value)
{
    caValue* t = term_insert_property(this, key);
    copy(value, t);
}

void Term::setIntProp(Symbol key, int i)
{
    caValue* t = term_insert_property(this, key);
    set_int(t, i);
}

void Term::setFloatProp(Symbol key, float f)
{
    caValue* t = term_insert_property(this, key);
    set_float(t, f);
}

void Term::setBoolProp(Symbol key, bool b)
{
    caValue* t = term_insert_property(this, key);
    set_bool(t, b);
}

void Term::setStringProp(Symbol key, std::string const& s)
{
    caValue* t = term_insert_property(this, key);
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

caValue* term_insert_property(Term* term, Symbol key)
{
    return hashtable_insert_int_key(&term->properties, key);
}

void term_set_property(Term* term, Symbol key, caValue* value)
{
    swap(value, term_insert_property(term, key));
}

caValue* term_get_property(Term* term, Symbol key)
{
    return hashtable_get_int_key(&term->properties, key);
}

void term_remove_property(Term* term, Symbol key)
{
    hashtable_remove_int_key(&term->properties, key);
}

void term_move_property(Term* from, Term* to, Symbol key)
{
    caValue* existing = term_get_property(from, key);
    if (existing == NULL)
        return;

    term_set_property(to, key, existing);
    term_remove_property(from, key);
}

caValue* term_get_input_property(Term* term, int inputIndex, Symbol key)
{
    Term::Input* info = term->inputInfo(inputIndex);
    if (info == NULL)
        return NULL;
    return hashtable_get_int_key(&info->properties, key);
}
caValue* term_insert_input_property(Term* term, int inputIndex, Symbol key)
{
    return hashtable_insert_int_key(&term->inputInfo(inputIndex)->properties, key);
}

bool term_get_bool_input_prop(Term* term, int inputIndex, Symbol key, bool defaultValue)
{
    caValue* value = term_get_input_property(term, inputIndex, key);
    if (value == NULL)
        return defaultValue;
    return as_bool(value);
}

const char* term_get_string_input_prop(Term* term, int inputIndex, Symbol key, const char* defaultValue)
{
    caValue* value = term_get_input_property(term, inputIndex, key);
    if (value == NULL)
        return defaultValue;
    return as_cstring(value);
}

int term_get_int_prop(Term* term, Symbol prop, int defaultValue)
{
    return term->intProp(prop, defaultValue);
}
const char* term_get_string_prop(Term* term, Symbol prop, const char* defaultValue)
{
    return term->stringProp(prop, defaultValue).c_str();
}
void term_set_int_prop(Term* term, Symbol prop, int value)
{
    term->setIntProp(prop, value);
}
void term_set_string_prop(Term* term, Symbol prop, const char* value)
{
    term->setStringProp(prop, value);
}
bool is_input_implicit(Term* term, int index)
{
    caValue* val = term_get_input_property(term, index, sym_Implicit);
    if (val == NULL)
        return false;
    return as_bool(val);
}
void set_input_implicit(Term* term, int index, bool implicit)
{
    set_bool(term_insert_input_property(term, index, sym_Implicit), true);
}
bool is_input_hidden(Term* term, int index)
{
    caValue* val = term_get_input_property(term, index, sym_Hidden);
    if (val == NULL)
        return false;
    return as_bool(val);
}
void set_input_hidden(Term* term, int index, bool hidden)
{
    set_bool(term_insert_input_property(term, index, sym_Hidden), true);
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
