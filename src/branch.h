// Copyright 2008 Andrew Fischer

#ifndef CIRCA_BRANCH_INCLUDED
#define CIRCA_BRANCH_INCLUDED

#include "common_headers.h"

#include "pointer_iterator.h"
#include "pointer_visitor.h"
#include "ref_list.h"
#include "term_namespace.h"

namespace circa {

struct Branch
{
    RefList _terms;

    TermNamespace names;

    // Points to a Branch which is our outer scope.
    Branch* outerScope;

    Branch() : outerScope(NULL) {}
    ~Branch();

    Branch& operator=(Branch const& b);

    int numTerms() const { return _terms.count(); }

    Term* get(int index) const { return _terms[index]; }
    Term* operator[](int index) const { return _terms[index]; }

    void append(Term* term);

    void removeTerm(std::string const& name);

    // Returns true if there is a term with the given name
    bool containsName(std::string const& name) const
    {
        return names.contains(name);
    }

    // Get the term with the given name.
    Term* getNamed(std::string const& name) const
    {
        return names[name];
    }

    // Find the first term with the given name binding.
    // This is kind of confusing. Might be good to revist this system.
    Term* findFirstBinding(std::string const& name) const;

    // Convenience syntax for getNamed
    Term* operator[](std::string const& name) const { return getNamed(name); }

    // Bind a name to a term
    void bindName(Term* term, std::string name);

    // Remap pointers
    void remapPointers(ReferenceMap const& map);

    // Visit pointers
    void visitPointers(PointerVisitor& visitor);

    void clear();

    // Evaluate this branch.
    void eval();

    // Compile the given statement, return the result term.
    Term* compile(std::string const& statement);

    // Evaluate the given statement, return the result term.
    Term* eval(std::string const& statement);

    // Start a sub-branch with the given name
    Branch& startBranch(std::string const& name);

    // Hosted functions
    static void* alloc(Term* type);
    static void dealloc(void* data);
    static void copy(Term* source, Term* dest);
    static void hosted_remap_pointers(Term* caller, ReferenceMap const& map);
    static void hosted_visit_pointers(Term* caller, PointerVisitor& visitor);
    static PointerIterator* start_pointer_iterator(Term* term);
};

bool is_branch(Term* term);
Branch& as_branch(Term* term);
std::string get_name_for_attribute(std::string attribute);
void duplicate_branch(Branch& source, Branch& dest);
void evaluate_file(Branch& branch, std::string const& filename);
PointerIterator* start_branch_iterator(Branch* branch);
PointerIterator* start_branch_pointer_iterator(Branch* branch);

Term* find_named(Branch* branch, std::string const& name);

bool branch_has_outer_scope(Branch& branch);
Term*& branch_get_outer_scope(Branch& branch);

void migrate_branch(Branch& replacement, Branch& target);
void reload_branch_from_file(Branch& branch);

} // namespace circa

#endif
