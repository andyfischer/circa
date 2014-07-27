// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "building.h"
#include "debug.h"
#include "hashtable.h"
#include "kernel.h"
#include "if_block.h"
#include "inspection.h"
#include "names.h"
#include "string_type.h"
#include "term.h"
#include "type.h"
#include "world.h"

#include "term.h"

namespace circa {

Term::Term(Block* _parent)
  : type(NULL),
    function(NULL),
    uniqueOrdinal(0),
    owningBlock(_parent),
    index(0),
    nestedContents(NULL)
{
    id = _parent->world->nextTermID++;

    set_hashtable(&properties);

    if (id == DEBUG_BREAK_ON_TERM)
        ca_debugger_break();
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
Term::input(int index)
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
Term::numInputs()
{
    return (int) this->inputs.size();
}

void
Term::inputsToList(TermList& out)
{
    out.resize(numInputs());
    for (int i=0; i < numInputs(); i++)
        out.setAt(i, input(i));
}

Term*
Term::dependency(int index)
{
    return term_dependency(this, index);
}

int
Term::numDependencies()
{
    return term_dependency_count(this);
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

void
Term::toString(caValue* out)
{
    to_string(term_value(this), out);
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

Term* alloc_term(Block* parent)
{
    Term* term = new Term(parent);
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

int term_dependency_count(Term* term)
{
    return term->numInputs() + 2;
}
Term* term_dependency(Term* term, int index)
{
    if (index == 0)
        return term->function;
    else if (index == 1)
        return declared_type_term(term);
    else
        return term->input(index - 2);
}
bool term_depends_on(Term* term, Term* termBeingUsed)
{
    for (int i=0; i < term_dependency_count(term); i++)
        if (term_dependency(term, i) == termBeingUsed)
            return true;
    return false;
}

Term* term_user(Term* term, int index)
{
    return term->users[index];
}
int user_count(Term* term)
{
    return term->users.length();
}

int real_user_count(Term* term)
{
    int count = 0;
    for (int i=0; i < user_count(term); i++) {
        Term* user = term_user(term, i);
        if (user->function == FUNCS.extra_output)
            continue;
        count++;
    }
    return count;
}

caValue* term_insert_property(Term* term, Symbol key)
{
    return hashtable_insert_symbol_key(&term->properties, key);
}

void term_set_property(Term* term, Symbol key, caValue* value)
{
    swap(value, term_insert_property(term, key));
}

caValue* term_get_property(Term* term, Symbol key)
{
    return hashtable_get_symbol_key(&term->properties, key);
}

void term_remove_property(Term* term, Symbol key)
{
    hashtable_remove_symbol_key(&term->properties, key);
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
    return hashtable_get_symbol_key(&info->properties, key);
}
caValue* term_insert_input_property(Term* term, int inputIndex, Symbol key)
{
    return hashtable_insert_symbol_key(&term->inputInfo(inputIndex)->properties, key);
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
bool term_get_bool_prop(Term* term, Symbol prop, bool defaultValue)
{
    return term->boolProp(prop, defaultValue);
}
void term_set_bool_prop(Term* term, Symbol prop, bool value)
{
    term->setBoolProp(prop, value);
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

void hide_from_source(Term* term)
{
    ca_assert(term != NULL);
    term->setBoolProp(sym_Hidden, true);
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

void format_global_id(Term* term, Value* out)
{
    if (!is_string(out))
        set_string(out, "");

    if (term == NULL)
        string_append(out, "null");
    else {
        string_append(out, "#");
        string_append(out, term->id);
    }
}

Term* parent_term(Term* term)
{
    if (term->owningBlock == NULL)
        return NULL;
    if (term->owningBlock->owningTerm == NULL)
        return NULL;

    return term->owningBlock->owningTerm;
}

Term* parent_term(Block* block)
{
    return block->owningTerm;
}

Term* parent_term(Term* term, int levels)
{
    for (int i=0; i < levels; i++) {
        term = parent_term(term);
        if (term == NULL)
            return NULL;
    }
    return term;
}

bool is_declared_state(Term* term)
{
    return term->function == FUNCS.declared_state;
}

bool uses_dynamic_dispatch(Term* term)
{
    return term->function == FUNCS.dynamic_method
        || term->function == FUNCS.func_apply
        || term->function == FUNCS.func_call
        || calls_function_by_value(term);
}

bool calls_function_by_value(Term* term)
{
    return !is_function(term->function);
}

Block* static_dispatch_block(Term* term)
{
    if (term->nestedContents != NULL && is_minor_block(term->nestedContents))
        return term->nestedContents;

    if (term->function == NULL || !is_function(term->function))
        return NULL;

    ca_assert(term->function->nestedContents != NULL);
    return term->function->nestedContents;
}

bool term_is_observable_for_special_reasons(Term* term)
{
    return (is_output_placeholder(term)
        || (term->function == FUNCS.loop_index)
        || (term->function == FUNCS.function_decl)
        || (is_for_loop(term->owningBlock) && is_input_placeholder(term) && term->index == 0)
        || (is_loop(term->owningBlock) && is_output_placeholder(term))
        || (term_get_bool_prop(term, sym_LocalStateResult, false)));
}

bool term_is_observable(Term* term)
{
    if (term_is_observable_for_special_reasons(term))
        return true;

    if (real_user_count(term) == 0)
        return false;

    // Future: Some more smarts here

    return true;
}

bool term_occurs_before_case_condition(Term* term)
{
    Term* condition = case_find_condition_check(term->owningBlock);
    if (condition == NULL)
        return false;

    return is_located_after(condition, term);
}

bool terms_are_in_different_switch_conditions(Term* left, Term* right)
{
    Block* commonBlock = find_common_parent_major(left->owningBlock, right->owningBlock);
    if (commonBlock == NULL)
        return false;

    if (!is_switch_block(commonBlock))
        return false;

    if (term_occurs_before_case_condition(left))
        return false;
    if (term_occurs_before_case_condition(right))
        return false;
        
    left = find_parent_term_in_block(left, commonBlock);
    right = find_parent_term_in_block(right, commonBlock);

    return left != right;
}

bool term_accesses_input_from_inside_loop(Term* term, Term* input)
{
    // Returns true if 1) "term" is inside a loop and 2) "input" is not inside that loop.
    // In other words, "term" will access "input"'s value multiple times.
    // (once per iteration)
    
    Block* enclosingLoop = find_enclosing_loop(term->owningBlock);
    if (enclosingLoop == NULL)
        return false;

    return !term_is_child_of_block(input, enclosingLoop);
}

bool term_is_observable_after(Term* term, Term* location)
{
    // Check if "term" can be observed after the "location".
    //
    // Typically, "location" uses "term" as an input, and we're trying to decide
    // whether the call at "location" can move/consume the value.

    if (term_is_observable_for_special_reasons(term))
        return true;

    for (int i=0; i < user_count(term); i++) {
        Term* user = term_user(term, i);

        if (user->function == FUNCS.nonlocal)
            return true;

        if (terms_are_in_different_switch_conditions(location, user))
            continue;

        if (term_accesses_input_from_inside_loop(user, term)) {

            bool userOnlyAccessesOnFirstIteration = is_input_placeholder(user)
                && user->numInputs() == 2
                && user->input(1) != term;

            if (!userOnlyAccessesOnFirstIteration)
                return true;
        }

        if (is_located_after(user, location))
            return true;
    }
 
    return false;
}

bool is_located_after(Term* term, Term* location)
{
    if (location == term)
        return false;

    Block* commonParent = find_common_parent(location->owningBlock, term->owningBlock);
    ca_assert(commonParent != NULL);

    Term* locationParent = find_parent_term_in_block(location, commonParent);
    Term* termParent = find_parent_term_in_block(term, commonParent);

    return termParent->index > locationParent->index;
}

bool term_uses_input_multiple_times(Term* term, Term* input)
{
    int useCount = 0;
    for (int i=0; i < term_dependency_count(term); i++)
        if (term_dependency(term, i) == input)
            useCount++;
    return useCount > 1;
}

} // namespace circa
