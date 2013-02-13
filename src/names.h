// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

struct NameSearch
{
    Block* block;

    // Search name.
    Value name;

    // Search position; the name search will look for bindings at this block index and
    // above.
    // If this is -1, it means the search position should be the end of the block.
    int position;

    // Unique ordinal value. The name search will only match terms with this ordinal
    // (the ordinal is used to distinguish terms that have the same name).
    // If this is -1, then the name search ignores the unique ordinal.
    int ordinal;

    // Lookup type, this may specify that we only search for type, function or module
    // values. This might also be LookupAny, to ignore the term's type.
    Symbol lookupType;

    // Whether to search parent blockes (if not found in target block).
    bool searchParent;
};

// Finds a name in this block or a visible parent block.
Term* find_name(Block* block,
                caValue* name,
                int position = -1,
                Symbol lookupType = sym_LookupAny);

// Finds a name in this block.
Term* find_local_name(Block* block,
                      caValue* name,
                      int position = -1,
                      Symbol lookupType = sym_LookupAny);

// Convenient overloads for using a string as a name
Term* find_name(Block* block,
                const char* name,
                int position = -1,
                Symbol lookupType = sym_LookupAny);

// Finds a name in this block.
Term* find_local_name(Block* block,
                      const char* name,
                      int position = -1,
                      Symbol lookupType = sym_LookupAny);

Term* find_name_at(Term* term, const char* name);
Term* find_name_at(Term* term, caValue* name);

// If the string is a qualified name (such as "a:b:c"), returns the index
// of the first colon. If the string isn't a qualified name then returns -1.
int name_find_qualified_separator(const char* name);

// If the name string has an ordinal (such as "a#1"), returns the ordinal value.
// Otherwise returns -1.
// endPos is the name's ending index (such as the one returned from
// name_find_qualified_separator);
int name_find_ordinal_suffix(const char* str, int* endPos);

// Get a named term from the global namespace.
Term* find_global(const char* name);

Block* get_parent_block(Block* block);
Term* get_parent_term(Term* term);
Term* get_parent_term(Block* block);
Term* get_parent_term(Term* term, int levels);
bool sym_is_reachable_from(Term* term, Block* block);
Block* find_first_common_block(Term* left, Term* right);
bool term_is_child_of_block(Term* term, Block* block);

// Get a name of 'term' which is valid in 'block'. This might simply return term's name,
// or if term is inside a namespace or object, this would return a colon-separated name.
std::string get_relative_name(Block* block, Term* term);
std::string get_relative_name_at(Term* location, Term* term);
void get_relative_name_as_list(Term* term, Block* relativeTo, caValue* nameOutput);
Term* find_from_relative_name_list(caValue* name, Block* relativeTo);

void update_unique_name(Term* term);
const char* get_unique_name(Term* term);

Term* find_from_unique_name(Block* block, const char* name);
Term* find_from_global_name(World* world, const char* globalName);

// Construct a global name for this term. May return :None if we couldn't create a global name.
void get_global_name(Term* term, caValue* nameOut);
void get_global_name(Block* term, caValue* nameOut);

Term* find_term_from_global_name(const char* name);

Type* find_type(World* world, const char* name);
Type* find_type_local(Block* block, const char* name);
Block* find_function(World* world, const char* name);
Block* find_function_local(Block* block, const char* name);
Block* find_module(World* world, const char* name);

#if 0
const char* name_to_string(Symbol name);
void name_to_string(Symbol name, String* string);
#endif
void qualified_name_get_first_section(caValue* name, caValue* prefixResult);
void qualified_name_get_remainder_after_first_section(caValue* name, caValue* suffixResult);

// Returns a name if there is already one with this string, otherwise returns None.
#if 0
Symbol existing_name_from_string(const char* str);
Symbol existing_name_from_string(const char* str, int len);

// Return a name from this string, adding it if necessary.
Symbol sym_from_string(const char* str);
Symbol sym_from_string(std::string const& str);
Symbol sym_from_string(caValue* str);

// Deallocate all interned names, this should be called at shutdown
void sym_dealloc_global_data();
#endif

} // namespace circa
