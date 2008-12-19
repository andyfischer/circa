// Copyright 2008 Paul Hodge

#ifndef CIRCA_BRANCH_INCLUDED
#define CIRCA_BRANCH_INCLUDED

#include "common_headers.h"

#include "pointer_visitor.h"
#include "term_namespace.h"

namespace circa {

class Branch
{
private:
    std::vector<Term*> _terms;

public:
    // This might be obsolete
    Term* owningTerm;

    // Points to a Branch term which is our outer scope.
    Term* outerScope;

    TermNamespace names;

    // 'permanent' means that this branch will not be deleted until the
    // entire program is shut down. This allows us to do a few things:
    //  - We're allowed to point to any term externally
    //  - We don't keep any record of external links
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

    // Hosted functions
    static void hosted_remap_pointers(Term* caller, ReferenceMap const& map);
    static void hosted_visit_pointers(Term* caller, PointerVisitor& visitor);
    static void hosted_update_owner(Term* term);
};

Branch& as_branch(Term* term);

void duplicate_branch(Branch* source, Branch* dest);

extern int DEBUG_CURRENTLY_INSIDE_BRANCH_DESTRUCTOR;

} // namespace circa

#endif
