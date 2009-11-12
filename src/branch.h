// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#ifndef CIRCA_BRANCH_INCLUDED
#define CIRCA_BRANCH_INCLUDED

#include "common_headers.h"

#include "ref_list.h"
#include "term_namespace.h"

namespace circa {

struct Branch
{
    RefList _terms;

    TermNamespace names;

    // Points to the Term which owns this branch as a value.
    Term* owningTerm;

    Branch() : owningTerm(NULL) {}
    ~Branch();

    int length() const { return _terms.length(); }

    Term* get(int index) const { return _terms[index]; }
    Ref& get(int index) { return _terms[index]; }
    Term* operator[](int index) const { assert(index <= length()); return _terms[index]; }
    Ref& operator[](int index) { assert(index <= length()); return _terms[index]; }

    // Get a term from a name binding.
    Term* get(std::string const& name) const { return names[name]; }
    Term* operator[](std::string const& name) const { return get(name); }

    Term* last() { if (length() == 0) return NULL; else return _terms[length()-1]; }

    // Find the index of the given term, returns -1 if not found.
    int findIndex(Term* term);

    // Find a term with the given name, returns -1 if not found.
    int findIndex(std::string const& name);

    // Returns true if there is a term with the given name
    bool contains(std::string const& name) const { return names.contains(name); }

    void append(Term* term);
    Term* appendNew();

    void insert(int index, Term* term);
    void moveToEnd(Term* term);

    void remove(Term* term);
    void remove(std::string const& name);
    void remove(int index);
    void removeNulls();

    void shorten(int newLength);

    // Find the first term with the given name binding.
    Term* findFirstBinding(std::string const& name) const;

    // Find the last term with the given name binding.
    Term* findLastBinding(std::string const& name) const;

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

// Assignment operator is disabled
private:
    Branch& operator=(Branch const& b) { return *this; }
};

// Hosted functions
namespace branch_t {
    void alloc(Term* type, Term* t);
    void dealloc(Term* type, Term* t);
    void assign(Term*, Term*);
    bool equals(Term*, Term*);
    void hosted_remap_pointers(Term* caller, ReferenceMap const& map);
}

bool is_branch(Term* term);
Branch& as_branch(Term* term);
bool is_namespace(Term* term);

std::string get_branch_source_filename(Branch& branch);
Branch* get_outer_scope(Branch& branch);

void duplicate_branch(Branch& source, Branch& dest);

void parse_script(Branch& branch, std::string const& filename);
void evaluate_script(Branch& branch, std::string const& filename);

Term* find_term_by_id(Branch& branch, unsigned int id);

bool reload_branch_from_file(Branch& branch, std::ostream& errors);
void persist_branch_to_file(Branch& branch);
std::string get_source_file_location(Branch& branch);

} // namespace circa

#endif
