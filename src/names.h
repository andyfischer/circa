// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

struct NameSearch
{
    Block* block;

    // Search name.
    Value name;

    // Position to start the name search, this can either be:
    //   integer - a term index.
    //   :Last   - last term in the block.
    Value position;

    // Unique ordinal value. The name search will only match terms with this ordinal
    // (the ordinal is used to distinguish terms that have the same name).
    // If this is -1, then the name search ignores the unique ordinal.
    int ordinal;

    // Lookup type, this may specify that we only search for type, function or module
    // values. This might also be LookupAny, to ignore the term's type.
    Symbol lookupType;

    // Whether to search parent blocks (if not found in target block).
    bool searchParent;
};

// Finds a name in this block or a visible parent block.
Term* find_name(Block* block,
                Value* name,
                Symbol lookupType = sym_LookupAny);

// Finds a name in this block.
Term* find_local_name(Block* block,
                      Value* name,
                      Symbol lookupType = sym_LookupAny);

// Convenient overloads for using a string as a name
Term* find_name(Block* block,
                const char* name,
                Symbol lookupType = sym_LookupAny);

// Finds a name in this block.
Term* find_local_name(Block* block,
                      const char* name,
                      Symbol lookupType = sym_LookupAny);

Term* find_local_name_at_position(Block* block, Value* name, Value* position);

Term* find_name_at(Term* term, const char* name);
Term* find_name_at(Term* term, Value* name);

// If the name string has an ordinal (such as "a#1"), returns the ordinal value.
// Otherwise returns -1.
// endPos is the name's ending index.
int name_find_ordinal_suffix(const char* str, int* endPos);

bool name_is_reachable_from(Term* term, Block* block);

Block* find_first_common_block(Term* left, Term* right);
bool term_is_child_of_block(Term* term, Block* block);

// Get a name of 'term' which is valid in 'block'. This might simply return term's name,
// or if term is inside a namespace or object, this would return a colon-separated name.
std::string get_relative_name(Block* block, Term* term);
std::string get_relative_name_at(Term* location, Term* term);
void get_relative_name_as_list(Term* term, Block* relativeTo, Value* nameOutput);
Term* find_from_relative_name_list(Value* name, Block* relativeTo);

void update_unique_name(Term* term);
Value* unique_name(Term* term);

Term* find_from_unique_name(Block* block, Value* name);

Type* find_type(Block* block, const char* name);
Block* find_function_local(Block* block, const char* name);

} // namespace circa
