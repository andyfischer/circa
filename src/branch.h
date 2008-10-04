// Copyright 2008 Paul Hodge

#ifndef CIRCA__BRANCH__INCLUDED
#define CIRCA__BRANCH__INCLUDED

#include "common_headers.h"

#include "term.h"
#include "term_namespace.h"

namespace circa {

class Branch
{
private:
    std::vector<Term*> terms;

public:
    TermNamespace names;

    Branch() {}
    ~Branch();

    int numTerms() const { return terms.size(); }

    Term* get(int index) const { return terms[index]; }
    Term* operator[](int index) const { return terms[index]; }

    void append(Term* term) { terms.push_back(term); }

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
    void bindName(Term* term, std::string name)
    {
        names.bind(term, name);
    }

    // Remap pointers
    void remapPointers(ReferenceMap const& map);

    void clear();
};

Branch* as_branch(Term* term);

} // namespace circa

#endif
