// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

struct NameSearch
{
    Branch* branch;

    // Search name.
    Name name;

    // Search position; the name search will look for bindings at this branch index and
    // above.
    // If this is -1, it means the search position should be the end of the branch.
    int position;

    // Unique ordinal value. The name search will only match terms with this ordinal
    // (the ordinal is used to distinguish terms that have the same name).
    // If this is -1, then the name search ignores the unique ordinal.
    int ordinal;

    // Lookup type, this may specify that we only search for type, function or module
    // values. This might also be LookupAny, to ignore the term's type.
    Name lookupType;

    // Whether to search parent branches (if not found in target branch).
    bool searchParent;
};

// Finds a name in this branch or a visible parent branch.
Term* find_name(Branch* branch,
                Name name,
                int position = -1,
                Name lookupType = name_LookupAny);

// Finds a name in this branch.
Term* find_local_name(Branch* branch,
                      Name name,
                      int position = -1,
                      Name lookupType = name_LookupAny);

// Convenient overloads for using a string as a name
Term* find_name(Branch* branch,
                const char* name,
                int position = -1,
                Name lookupType = name_LookupAny);

// Finds a name in this branch.
Term* find_local_name(Branch* branch,
                      const char* name,
                      int position = -1,
                      Name lookupType = name_LookupAny);

Term* find_name_at(Term* term, const char* name);
Term* find_name_at(Term* term, Name name);

// If the string is a qualified name (such as "a:b:c"), returns the index
// of the first colon. If the string isn't a qualified name then returns -1.
int name_find_qualified_separator(const char* name);

// If the name string has an ordinal (such as "a#1"), returns the ordinal value.
// Otherwise returns -1.
// endPos is the name's ending index (such as the one returned from
// name_find_qualified_separator);
int name_find_ordinal_suffix(const char* str, int* endPos);

// Get a named term from the global namespace.
Term* get_global(Name name);
Term* get_global(const char* name);

Branch* get_parent_branch(Branch* branch);
Term* get_parent_term(Term* term);
Term* get_parent_term(Term* term, int levels);
bool name_is_reachable_from(Term* term, Branch* branch);
Branch* find_first_common_branch(Term* left, Term* right);
bool term_is_child_of_branch(Term* term, Branch* branch);

// Get a name of 'term' which is valid in 'branch'. This might simply return term's name,
// or if term is inside a namespace or object, this would return a colon-separated name.
std::string get_relative_name(Branch* branch, Term* term);
std::string get_relative_name_at(Term* location, Term* term);

void update_unique_name(Term* term);
const char* get_unique_name(Term* term);

Term* find_from_unique_name(Branch* branch, const char* name);

// Attempts to find a global name for the term. If successful, returns true
// and writes the result to 'name'. If unsuccessful (for example, if the
// Branch is locally-allocated), returns false.
bool find_global_name(Term* term, std::string& name);

// Convenience function, calls find_global_name and returns a blank string if
// it wasn't found.
std::string find_global_name(Term* term);

Term* find_term_from_global_name(const char* name);

bool name_is_valid(Name index);
const char* name_to_string(Name name);
void name_to_string(Name name, String* string);
Name qualified_name_get_first_section(Name name);
Name qualified_name_get_remainder_after_first_section(Name name);

Name as_name(caValue* tv);
void set_name(caValue* tv, Name name);

// Returns a name if there is already one with this string, otherwise returns None.
Name existing_name_from_string(const char* str);

// Return a name from this string, adding it if necessary.
Name name_from_string(const char* str);
Name name_from_string(std::string const& str);
Name name_from_string(caValue* str);

// Deallocate all interned names, this should be called at shutdown
void name_dealloc_global_data();

} // namespace circa
