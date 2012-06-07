// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

typedef int Name;

enum NameLookupType {
    NAME_LOOKUP_ANY = 1,
    NAME_LOOKUP_TYPE = 2,
    NAME_LOOKUP_FUNCTION = 3,
    NAME_LOOKUP_MODULE = 4
};

// Finds a name in this branch or a visible parent branch.
Term* find_name(Branch* branch,
                Name name,
                int location = -1,
                NameLookupType lookupType = NAME_LOOKUP_ANY);

// Finds a name in this branch.
Term* find_local_name(Branch* branch,
                      Name name,
                      int location = -1,
                      NameLookupType lookupType = NAME_LOOKUP_ANY);

// Convenient overloads for using a string as a name
Term* find_name(Branch* branch,
                const char* name,
                int location = -1,
                NameLookupType lookupType = NAME_LOOKUP_ANY);

// Finds a name in this branch.
Term* find_local_name(Branch* branch,
                      const char* name,
                      int location = -1,
                      NameLookupType lookupType = NAME_LOOKUP_ANY);

Term* find_name_at(Term* term, const char* name);

// If the string is a qualified name (such as "a:b:c"), returns the index
// of the first colon. If the string isn't a qualified name then returns -1.
int find_qualified_name_separator(const char* name);

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
Name name_get_namespace_first(Name name);
Name name_get_namespace_rr(Name name);

Name as_name(caValue* tv);
void set_name(caValue* tv, Name name);

Name name_from_string(const char* str);
Name name_from_string(caValue* str);

// Deallocate all interned names, this should be called at shutdown
void name_dealloc_global_data();

} // namespace circa
