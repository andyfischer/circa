// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "block.h"
#include "term_list.h"
#include "tagged_value.h"
#include "term_source_location.h"

namespace circa {

struct Term
{
    Value value;

    int id; // Globally unique ID.

    // A Type that statically describes our type.
    Type* type;

    struct Input {
        Term* term;
        Value properties;

        Input();
        Input(Term* t);
    };

    typedef std::vector<Input> InputList;

    // Input terms
    InputList inputs;

    // Our function: the thing that takes our inputs and produces a value.
    Term* function;

    const char* name();

    Value nameValue;

    // A name which is unique across this block.
    struct UniqueName
    {
        Value name;
        Value base;
        int ordinal;
        UniqueName() : ordinal(0) {}
    };

    UniqueName uniqueName;

    // An ordinal value that is locally unique among terms with the same name.
    int uniqueOrdinal;

    // The block that owns this term. May be NULL.
    Block* owningBlock;

    // The index that this term currently holds inside owningBlock.
    int index;

    // Code which is nested inside this term. This object is created on-demand.
    Block* nestedContents;

    // Dynamic properties
    Value properties;

    // Terms which are using this term as an input.
    TermList users;

    // Location in textual source code.
    TermSourceLocation sourceLoc;

    Term(Block* block);

    const char* nameStr();

    Term* input(int index);
    Input* inputInfo(int index);
    int numInputs();

    void inputsToList(TermList& out);

    // In this context, the 'dependencies' include the function term and all input
    // terms. So, this is the same as the input list, with the function inserted
    // at element 0. This is more convenient in some situations.
    Term* dependency(int index);
    int numDependencies();
    void setDependency(int index, Term* term);

    // Shorthand for nested_contents()
    Block* contents();

    // Shorthand for nested_contents()[index]
    Term* contents(int index);

    // Shorthand for nested_contents()[name]
    Term* contents(const char* name);

    void toString(Value* out);

    bool hasProperty(Symbol key);
    void removeProperty(Symbol key);
    Value* getProp(Symbol key);

    int intProp(Symbol key, int defaultValue);
    float floatProp(Symbol key, float defaultValue);
    bool boolProp(Symbol key, bool defaultValue);
    std::string stringProp(Symbol key, const char* defaultValue);

    void setProp(Symbol key, Value* value);
    void setIntProp(Symbol key, int i);
    void setFloatProp(Symbol key, float f);
    void setBoolProp(Symbol key, bool b);
    void setStringProp(Symbol key, std::string const& s);

    void dump();
    Block* parent();
};

// Allocate a new Term object.
Term* alloc_term(Block* parent);
void dealloc_term(Term*);

int term_dependency_count(Term* term);
Term* term_dependency(Term* term, int i);
bool term_depends_on(Term* term, Term* termBeingUsed);

Term* term_user(Term* term, int index);
int user_count(Term* term);

bool is_located_after(Term* location, Term* term);

// Fetches a term property, creating it if it doesn't exist.
Value* term_insert_property(Term* term, Symbol key);

// Fetches a term property.
Value* term_get_property(Term* term, Symbol key);

// Assigns a property with the given name and value. The argument 'value' is consumed.
void term_set_property(Term* term, Symbol key, Value* value);

// Removes a term property.
void term_remove_property(Term* term, Symbol key);

// Removes the property with the given name from 'source', and assigns that
// property to 'dest'. Has no effect if 'source' does not have the property.
void term_move_property(Term* source, Term* dest, Symbol key);

// Fetches an input property.
Value* term_get_input_property(Term* term, int inputIndex, Symbol key);
Value* term_insert_input_property(Term* term, int inputIndex, Symbol key);
bool term_get_bool_input_prop(Term* term, int inputIndex, Symbol key, bool defaultValue);
const char* term_get_string_input_prop(Term* term, int inputIndex, Symbol key,
    const char* defaultValue);

int term_get_int_prop(Term* term, Symbol prop, int defaultValue);
void term_set_int_prop(Term* term, Symbol prop, int value);
const char* term_get_string_prop(Term* term, Symbol prop, const char* defaultValue);
void term_set_string_prop(Term* term, Symbol prop, const char* value);
bool term_get_bool_prop(Term* term, Symbol prop, bool defaultValue);
void term_set_bool_prop(Term* term, Symbol prop, bool value);

// Specific helper functions for input properties.
bool is_input_implicit(Term* term, int index);
void set_input_implicit(Term* term, int index, bool implicit);
bool is_input_hidden(Term* term, int inputIndex);
void set_input_hidden(Term* term, int inputIndex, bool hidden);

// Mark the given term as hidden from source reproduction.
void hide_from_source(Term* term);

Value* term_name(Term* term);
Value* term_value(Term* term);
Block* term_function(Term* term);
bool is_type(Term* term);
bool is_function(Term* term);
Type* as_type(Term* term);
int term_line_number(Term* term);
void format_global_id(Term* term, Value* out);

Term* parent_term(Term* term);
Term* parent_term(Block* block);
Term* parent_term(Term* term, int levels);

bool is_declared_state(Term* term);
bool uses_dynamic_dispatch(Term* term);

// Returns a (statically-known) block that will be used for this term's evaluation.
// May return NULL.
Block* term_get_dispatch_block(Term* term);

bool calls_function_by_value(Term* term);
Block* static_dispatch_block(Term* term);

bool term_accesses_input_from_inside_loop(Term* term, Term* input);
bool term_is_observable_for_special_reasons(Term* term);
bool term_is_observable(Term* term);
bool term_used_by_nonlocal(Term* term);
bool term_is_observable_after(Term* term, Term* location);
bool is_located_after(Term* location, Term* term);
bool term_uses_input_multiple_times(Term* term, Term* input);
bool term_needs_no_evaluation(Term* term);
bool has_static_value(Term* term);
bool can_consume_term_result(Term* input, Term* user);

} // namespace circa
