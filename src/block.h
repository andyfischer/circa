// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "term_list.h"
#include "term_namespace.h"

namespace circa {

struct BlockOverrides
{
    EvaluateFunc evaluate;
    SpecializeTypeFunc specializeType;
    FormatSourceFunc formatSource;
    PostCompileFunc postCompile;

    BlockOverrides() :
      evaluate(NULL),
      specializeType(NULL),
      formatSource(NULL),
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
    // CircaObject header;

    // Globally unique ID. Used for debugging.
    int id;

    // List of content terms. This block owns all the Term objects in this list.
    TermList _terms;

    // Symbol bindings.
    TermNamespace names;

    // Points to the Term which owns this block as a value.
    Term* owningTerm;

    // Whether this block is "in progress". Certain cleanup actions are suspended
    // while in this state.
    bool inProgress;

    // If this block has any static errors, then they are listed here. If there
    // are no errors then this value is null.
    // If this has a list, each element has structure:
    //  [0] int index
    //  [1] string type
    //  [2] int inputIndex (only used for errors related to inputs)
    Value staticErrors;

    // Dictionary with additional metadata.
    Value properties;

    BlockOverrides overrides;
    FunctionAttrs functionAttrs;

    Block();
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
    Term* findFirstBinding(caValue* name);

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
    void bindName(Term* term, caValue* name);

    // Remap pointers
    void remapPointers(TermMap const& map);

    // Compile the given statement, return the result term.
    Term* compile(std::string const& statement);

    std::string toString();
    void dump();
    Term* owner();
    Block* parent();

private:
    // Disallow copy constructor
    Block(Block const&) { internal_error(""); }
    Block& operator=(Block const&) { internal_error(""); return *this; }
};

void block_setup_type(Type* type);
void assert_valid_block(Block const* obj);

Block* alloc_block();

bool has_nested_contents(Term* term);
Block* make_nested_contents(Term* term);
Block* nested_contents(Term* term);
Term* block_get_function_term(Block* block);

void remove_nested_contents(Term* term);

// Exactly the same as 'nested_contents' but conventionally used for functions:
Block* function_contents(Term* func);

// Insert this existing block as the nested contents for this term.
void block_graft_replacement(Block* target, Block* replacement);

caValue* block_get_source_filename(Block* block);

Block* get_outer_scope(Block* block);

// Delete this term and remove it from its owning block.
void erase_term(Term* term);

// Delete the contents of 'block'.
void clear_block(Block* block);

// Deep copy of a block. Handles the migration of inner term references.
void duplicate_block(Block* source, Block* dest);

// Compile the string as a statement list. Appends new terms to the block and
// returns the last new term.
Term* compile(Block* block, const char* str);

Symbol load_script(Block* block, const char* filename);

void post_module_load(Block* block);

// Create an include() call that loads the given file. Returns the included
// block.
Block* include_script(Block* block, const char* filename);

// Create a load_script() call that loads the given file. The load_script
// function doesn't evaluate the contents when called. Returns the included
// block.
Block* load_script_term(Block* block, const char* filename);

void remove_nulls(Block* block);

// Return the block's override func. May be NULL.
EvaluateFunc get_override_for_block(Block* block);

Term* find_term_by_id(Block* block, int id);

std::string get_source_file_location(Block* block);

// Block properties
caValue* block_get_property(Block* block, Symbol name);
caValue* block_insert_property(Block* block, Symbol name);
void block_remove_property(Block* block, Symbol name);

bool block_get_bool_prop(Block* block, Symbol name, bool defaultValue);
void block_set_bool_prop(Block* block, Symbol name, bool value);

// Convenience functions for accessing block properties.
bool block_is_evaluation_empty(Block* block);
void block_set_evaluation_empty(Block* block, bool empty);
bool block_has_effects(Block* block);
void block_set_has_effects(Block* block, bool hasEffects);
int block_locals_count(Block* block);

Type* get_input_type(Block* block, int index);
Type* get_output_type(Block* block, int index);

void block_set_evaluate_func(Block* block, EvaluateFunc eval);
void block_set_specialize_type_func(Block* block, SpecializeTypeFunc specializeFunc);
void block_set_format_source_func(Block* block, FormatSourceFunc formatSource);
void block_set_post_compile_func(Block* block, PostCompileFunc postCompile);
void block_set_function_has_nested(Block* block, bool hasNestedContents);

void block_check_invariants(caValue* result, Block* block);
bool block_check_invariants_print_result(Block* block, std::ostream& out);

void block_link_missing_functions(Block* block, Block* source);

bool block_is_child_of(Block* possibleChild, Block* possibleParent);

} // namespace circa
