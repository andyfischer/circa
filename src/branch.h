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
    // Our owning term
    Term* owningTerm;

    // Points to a Branch which is our outer scope.
    Branch* outerScope;

    TermNamespace names;

    // 'permanent' means that this branch will not be deleted until the
    // entire program is shut down.
    bool permanent;

    Branch() : owningTerm(NULL), outerScope(NULL), permanent(false) {}
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

    // Return a term with the given name. If this branch doesn't contain the name,
    // we will search upwards.
    Term* findNamed(std::string const& name) const;

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
    static void hosted_update_owner(Term* term);
    static PointerIterator* start_pointer_iterator(Term* term);
};

Branch& as_branch(Term* term);
void duplicate_branch(Branch& source, Branch& dest);
void migrate_branch(Branch& original, Branch& replacement);
void evaluate_file(Branch& branch, std::string const& filename);
void reload_branch_from_file(Branch& branch);

extern int DEBUG_CURRENTLY_INSIDE_BRANCH_DESTRUCTOR;

} // namespace circa

#endif
