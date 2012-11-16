// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "circa/circa.h"

#include "common_headers.h"

#include "names.h"
#include "gc.h"
#include "list.h"
#include "tagged_value.h"
#include "term_list.h"
#include "term_namespace.h"

namespace circa {

struct Branch
{
    CircaObject header;

    // Globally unique ID. Used for debugging.
    int id;

    // List of content terms. This branch owns all the Term objects in this list.
    TermList _terms;

    // Name bindings.
    TermNamespace names;

    // Points to the Term which owns this branch as a value.
    Term* owningTerm;

    // Monotonically increasing version number.
    int version;

    // Whether this branch is "in progress". Certain cleanup actions are suspended
    // while in this state.
    bool inProgress;

    // Variant value describing where this branch came from. 
    //   If the branch came from a file, then the value will be of format:
    //     [:file, String filename, int nullable_timestamp]
    Value origin;

    // If this branch has any static errors, then they are listed here. If there
    // are no errors then this value is null.
    // If this has a list, each element has structure:
    //  [0] int index
    //  [1] string type
    //  [2] int inputIndex (only used for errors related to inputs)
    Value staticErrors;

    // If this branch is used as a function, this dict may contain extra metadata.
    Value functionAttrs;

    // Compound type object describing our inlined state. May be NULL.
    Type* stateType;

    // Evaluation advice
    bool emptyEvaluation;

    // Whether this branch is effectual or contains any effectual calls.
    bool effectual;

    // Compiled interpreter instructions.
    Value bytecode;

    Branch();
    ~Branch();

    int length();

    Term* get(int index);
    Term* getSafe(int index);

    // Get an index counting from the end; equivalent to get(length() - index - 1)
    Term* getFromEnd(int index);

    Term* operator[](int index) { return get(index); }

    // Get a term from a name binding.
    inline Term* get(std::string const& name) {
        return find_local_name(this, circa_to_name(name.c_str()));
    }
    inline Term* getNamed(const char* name) {
        return find_local_name(this, circa_to_name(name));
    }

    inline Term* operator[](std::string const& name) {
        return find_local_name(this, circa_to_name(name.c_str()));
    }

    // Returns true if there is a term with the given name
    bool contains(std::string const& name);

    int getIndex(Term* term);

    Term* last();

    // Find the first term with the given name binding.
    Term* findFirstBinding(Name name);

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
    void bindName(Term* term, Name name);

    // Remap pointers
    void remapPointers(TermMap const& map);

    // Compile the given statement, return the result term.
    Term* compile(std::string const& statement);

    // Evaluate the given statement, return the result value.
    Term* eval(std::string const& statement);

    std::string toString();
    void dump();
    Term* owner();
    Branch* parent();

private:
    // Disallow copy constructor
    Branch(Branch const&) { internal_error(""); }
    Branch& operator=(Branch const&) { internal_error(""); return *this; }
};

void branch_setup_type(Type* type);
void assert_valid_branch(Branch const* obj);

Branch* alloc_branch_gc();

bool is_namespace(Term* term);
bool is_namespace(Branch* branch);

bool has_nested_contents(Term* term);
Branch* nested_contents(Term* term);
void remove_nested_contents(Term* term);

// Insert this existing branch as the nested contents for this term.
void branch_graft_as_nested_contents(Term* term, Branch* branch);

caValue* branch_get_source_filename(Branch* branch);
Branch* get_outer_scope(Branch* branch);

// Delete this term and remove it from its owning branch.
void erase_term(Term* term);

// Delete the contents of 'branch'.
void clear_branch(Branch* branch);

void duplicate_branch(Branch* source, Branch* dest);

Name load_script(Branch* branch, const char* filename);
void post_module_load(Branch* branch);

// Create an include() call that loads the given file. Returns the included
// branch.
Branch* include_script(Branch* branch, const char* filename);

// Create a load_script() call that loads the given file. The load_script
// function doesn't evaluate the contents when called. Returns the included
// branch.
Branch* load_script_term(Branch* branch, const char* filename);

Term* find_term_by_id(Branch* branch, int id);

std::string get_source_file_location(Branch* branch);

bool branch_get_function_attr_bool(Branch* branch, Name attr);

// Returns a List pointer if the branch has a file origin, NULL if not.
List* branch_get_file_origin(Branch* branch);

// Checks Branch.origin, and checks the modified time of 'filename'. If the origin
// does not match the file's modified time, then we return true and update the
// branch's origin. So, if this function true then the branch should be reloaded.
bool check_and_update_file_origin(Branch* branch, const char* filename);

// Using the branch origin, this checks the filesystem to see if there is a new
// version of this branch available. If so, the new version is loaded and returned.
// If not, the exisiting Branch is returned.
Branch* load_latest_branch(Branch* branch);

void branch_check_invariants(caValue* result, Branch* branch);
bool branch_check_invariants_print_result(Branch* branch, std::ostream& out);

// Update the branch's stateType. Should be called after the code is changed in a way
// that could add/remove declared state.
void branch_update_state_type(Branch* branch);

void branch_link_missing_functions(Branch* branch, Branch* source);

} // namespace circa
