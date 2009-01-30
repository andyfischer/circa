// Copyright 2008 Andrew Fischer

#ifndef CIRCA_BRANCH_INCLUDED
#define CIRCA_BRANCH_INCLUDED

#include "common_headers.h"

#include "pointer_iterator.h"
#include "pointer_visitor.h"
#include "term_namespace.h"

namespace circa {

class Branch
{
private:
    std::vector<Term*> _terms;

public:
    // Points to a Branch which is our outer scope.
    Branch* outerScope;

    TermNamespace names;

    // 'permanent' means that this branch will not be deleted until the
    // entire program is shut down.
    bool permanent;

    Branch() : outerScope(NULL), permanent(false) {}
    ~Branch();

    int numTerms() const { return _terms.size(); }

    Term* get(int index) const { return _terms[index]; }
    Term* operator[](int index) const { return _terms[index]; }

    void append(Term* term);

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
    static void hosted_remap_pointers(Term* caller, ReferenceMap const& map);
    static void hosted_visit_pointers(Term* caller, PointerVisitor& visitor);
    static PointerIterator* start_pointer_iterator(Term* term);
};

Branch& as_branch(Term* term);
void duplicate_branch(Branch& source, Branch& dest);
void migrate_branch(Branch& replacement, Branch& target);
void evaluate_file(Branch& branch, std::string const& filename);
void reload_branch_from_file(Branch& branch);
PointerIterator* start_branch_iterator(Branch* branch);
PointerIterator* start_branch_pointer_iterator(Branch* branch);
PointerIterator* start_branch_control_flow_iterator(Branch* branch);

Term* find_named(Branch* branch, std::string const& name);

} // namespace circa

#endif
