// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "common_headers.h"

#include "names.h"
#include "ref_list.h"
#include "term_namespace.h"

namespace circa {

struct Branch
{
    RefList _terms;

    TermNamespace names;

    // Points to the Term which owns this branch as a value.
    Term* owningTerm;

    int _refCount;

    Branch();
    ~Branch();

    int length() const;

    Term* get(int index) const;
    Term* operator[](int index) const { return get(index); }

    // Get a term from a name binding.
    inline Term* get(std::string const& name) const { return get_named(*this, name); }
    inline Term* operator[](std::string const& name) const { return get_named(*this, name); }

    // Returns true if there is a term with the given name
    bool contains(std::string const& name) const;

    int getIndex(Term* term) const;
    int debugFindIndex(Term* term) const;

    Term* last() const;

    // Find the first term with the given name binding.
    Term* findFirstBinding(std::string const& name) const;

    // Find the last term with the given name binding.
    Term* findLastBinding(std::string const& name) const;

    // Find a term with the given name, returns -1 if not found.
    int findIndex(std::string const& name) const;

    void set(int index, Term* term);
    void setNull(int index);

    void insert(int index, Term* term);

    void append(Term* term);
    Term* appendNew();

    void moveToEnd(Term* term);

    void remove(Term* term);
    void remove(std::string const& name);
    void remove(int index);
    void removeNulls();
    void shorten(int newLength);

    // Bind a name to a term
    void bindName(Term* term, std::string name);

    // Remap pointers
    void remapPointers(ReferenceMap const& map);

    void clear();

    // Compile the given statement, return the result term.
    Term* compile(std::string const& statement);

    // Evaluate the given statement, return the result term.
    Term* eval(std::string const& statement);

    std::string toString();

private:
    // Disabled calls
    Branch(Branch const& copy) {}
    Branch& operator=(Branch const& b) { return *this; }
};

// Hosted functions
namespace branch_t {
    void initialize(Type*, TaggedValue* value);
    void release(TaggedValue* value);
    void assign(TaggedValue*, TaggedValue*);
    void cast(Type*, TaggedValue* source, TaggedValue* dest);
    bool cast_possible(Type*, TaggedValue* value);
    bool equals(TaggedValue*, TaggedValue*);
    TaggedValue* get_index(TaggedValue* value, int index);
    int num_elements(TaggedValue* value);

    void branch_copy(Branch& source, Branch& dest);
    void assign(Branch& source, Branch& dest);
}

bool is_branch(TaggedValue* term);
Branch& as_branch(TaggedValue* term);

bool is_branch_based_type(Term* type);
bool is_branch_based_type(Type* type);
void initialize_branch_based_type(Term* term);

bool is_namespace(Term* term);
bool is_namespace(Branch& branch);

std::string get_branch_source_filename(Branch& branch);
Branch* get_outer_scope(Branch const& branch);

void duplicate_branch(Branch& source, Branch& dest);

void parse_script(Branch& branch, std::string const& filename);
void evaluate_script(Branch& branch, std::string const& filename);

Term* find_term_by_id(Branch& branch, unsigned int id);

void persist_branch_to_file(Branch& branch);
std::string get_source_file_location(Branch& branch);

bool branch_check_invariants(Branch& branch, std::ostream* output=NULL);

} // namespace circa
