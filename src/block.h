// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "term_list.h"
#include "term_namespace.h"

namespace circa {

struct BlockOverrides
{
    SpecializeTypeFunc specializeType;
    PostCompileFunc postCompile;

    BlockOverrides() :
      specializeType(NULL),
      postCompile(NULL)
    {}
};

struct FunctionAttrs
{
    // Whether a term that uses this Block as its function should have a nestedContents block.
    bool hasNestedContents;

    FunctionAttrs() :
      hasNestedContents(false)
    {}
};

struct Block
{
    // Globally unique ID.
    int id;

    // List of content terms. This block owns all the Term objects in this list.
    TermList _terms;

    // Name bindings.
    TermNamespace names;

    // The term which owns this block (as term->nestedContents).
    Term* owningTerm;

    // Whether this block is "in progress". Certain cleanup actions are suspended
    // while in this state.
    bool inProgress;

    // Dictionary with additional metadata.
    Value properties;

    BlockOverrides overrides;
    FunctionAttrs functionAttrs;

    World* world;

    Block(World* world = NULL);
    ~Block();

    int length();

    Term* get(int index);
    Term* getSafe(int index);

    // Get an index counting from the end; equivalent to get(length() - index - 1)
    Term* getFromEnd(int index);

    Term* operator[](int index) { return get(index); }

    // Get a term from a name binding.
    Term* get(std::string const& name);
    Term* getNamed(const char* name);
    Term* operator[](std::string const& name);

    // Returns true if there is a term with the given name
    bool contains(std::string const& name);

    int getIndex(Term* term);

    Term* last();

    // Find the first term with the given name binding.
    Term* findFirstBinding(Value* name);

    void insert(int index, Term* term);
    void append(Term* term);
    Term* appendNew();

    void set(int index, Term* term);
    void setNull(int index);

    void move(Term* term, int index);
    void moveToEnd(Term* term);

    void remove(int index);
    void remove(std::string const& name);
    void removeNulls();
    void removeNameBinding(Term* term);
    void shorten(int newLength);
    void clear();

    // Bind a name to a term
    void bindName(Term* term, Value* name);

    // Remap pointers
    void remapPointers(TermMap const& map);

    // Compile the given statement, return the result term.
    Term* compile(const char* statement);

    void dump();
    void dump_with_props();
    Term* owner();
    Block* parent();
    const char* name();

private:
    // Disallow copy constructor
    Block(Block const&) { internal_error(""); }
    Block& operator=(Block const&) { internal_error(""); return *this; }
};

void block_setup_type(Type* type);
void assert_valid_block(Block const* obj);

Block* alloc_block(World* world);

bool has_nested_contents(Term* term);
Block* make_nested_contents(Term* term);
Block* nested_contents(Term* term);
Term* block_get_function_term(Block* block);
Value* block_name(Block* block);
void remove_nested_contents(Term* term);

Term* get_output_placeholder(Block* block, int index);
int count_input_placeholders(Block* block);
int count_output_placeholders(Block* block);
bool has_variable_args(Block* block);

// Insert this existing block as the nested contents for this term.
void block_graft_replacement(Block* target, Block* replacement);

Block* get_outer_scope(Block* block);
Block* get_parent_block(Block* block);
Block* get_parent_block_stackwise(Block* block);
Block* find_enclosing_loop(Block* block);
Block* find_enclosing_major_block(Block* block);
Block* find_enclosing_major_block(Term* term);

Block* find_common_parent(Block* a, Block* b);
Block* find_common_parent(Term* a, Term* b);
Block* find_common_parent_major(Block* a, Block* b);

// Search upwards starting at 'term', and returns the parent (or the term itself) found
// in 'block'. Returns NULL if not found.
Term* find_parent_term_in_block(Term* term, Block* block);

bool is_case_block(Block* block);
bool is_switch_block(Block* block);

bool is_for_loop(Block* block);
bool is_while_loop(Block* block);
bool is_loop(Block* block);

// Delete this term and remove it from its owning block.
void erase_term(Term* term);

// Delete the contents of 'block'.
void clear_block(Block* block);

// Compile the string as a statement list. Appends new terms to the block and
// returns the last new term.
Term* compile(Block* block, const char* str);

void load_script_from_text(Block* block, const char* text);
void load_script(Block* block, const char* filename);

void post_module_load(Block* block);

void remove_nulls(Block* block);

Term* find_term_by_id(Block* block, int id);

void get_source_file_location(Block* block, Value* out);

// Block properties
bool block_has_property(Block* block, Symbol name);
Value* block_get_property(Block* block, Symbol name);
Value* block_insert_property(Block* block, Symbol name);
void block_remove_property(Block* block, Symbol name);

Value* block_get_source_filename(Block* block);
Value* block_get_static_errors(Block* block);

bool block_get_bool_prop(Block* block, Symbol name, bool defaultValue);
void block_set_bool_prop(Block* block, Symbol name, bool value);
Symbol block_get_symbol_prop(Block* block, Symbol name, Symbol defaultValue);
void block_set_symbol_prop(Block* block, Symbol name, Symbol value);

// Convenience functions for accessing block properties.
bool block_is_evaluation_empty(Block* block);
void block_set_evaluation_empty(Block* block, bool empty);
bool block_has_effects(Block* block);
void block_set_has_effects(Block* block, bool hasEffects);

Type* get_input_type(Block* block, int index);
Type* get_output_type(Block* block, int index);

void block_set_specialize_type_func(Block* block, SpecializeTypeFunc specializeFunc);
void block_set_post_compile_func(Block* block, PostCompileFunc postCompile);
void block_set_function_has_nested(Block* block, bool hasNestedContents);

void block_check_invariants(Value* result, Block* block);
bool block_check_invariants_print_result(Block* block, Value* out);

void block_link_missing_functions(Block* block, Block* source);

bool block_is_child_of(Block* possibleChild, Block* possibleParent);

} // namespace circa
