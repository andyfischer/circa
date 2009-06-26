// Copyright 2008 Andrew Fischer

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

    Branch();
    ~Branch();

    Branch& operator=(Branch const& b);

    int length() const { return _terms.length(); }

    Term* get(int index) const { return _terms[index]; }
    Ref& get(int index) { return _terms[index]; }
    Term* operator[](int index) const { return _terms[index]; }
    Ref& operator[](int index) { return _terms[index]; }

    // Get a term from a name binding.
    Term* get(std::string const& name) const { return names[name]; }
    Term* operator[](std::string const& name) const { return get(name); }

    // Returns true if there is a term with the given name
    bool contains(std::string const& name) const { return names.contains(name); }

    void append(Term* term);

    void insert(int index, Term* term);

    void remove(Term* term);
    void remove(std::string const& name);
    void remove(int index);
    void removeNulls();

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

    // Hosted functions
    static void* alloc(Term* type);
    static void dealloc(void* data);
    static void assign(Term* source, Term* dest);
    static void hosted_remap_pointers(Term* caller, ReferenceMap const& map);
};

bool is_branch(Term* term);
Branch& as_branch(Term* term);
std::string get_name_for_attribute(std::string attribute);
Branch* get_outer_scope(Branch& branch);

Branch& create_branch(Branch* owner, std::string const& name="");

void duplicate_branch(Branch& source, Branch& dest);

void parse_script(Branch& branch, std::string const& filename);
void evaluate_script(Branch& branch, std::string const& filename);

Term* find_term_by_id(Branch& branch, unsigned int id);
Term* find_named(Branch* branch, std::string const& name);

void reload_branch_from_file(Branch& branch);
void persist_branch_to_file(Branch& branch);

} // namespace circa

#endif
