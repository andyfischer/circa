#ifndef CIRCA__BRANCH__INCLUDED
#define CIRCA__BRANCH__INCLUDED

#include "common_headers.h"

#include "term.h"
#include "term_namespace.h"

namespace circa {

struct Branch
{
    TermList terms;
    TermNamespace names;

    Branch();

    void append(Term* term);

    // Returns true if there is a term with the given name
    bool containsName(std::string const& name) const;

    // Get the term with the given name.
    Term* getNamed(std::string const& name) const;

    // Return a term with the given name. If this branch doesn't contain the name,
    // we will search upwards.
    Term* findNamed(std::string const& name) const;

    // Bind a name to a term
    void bindName(Term* term, string name);

    // Remap pointers
    void remapPointers(TermMap const& map);

    void clear();
};

Branch* as_branch(Term* term);

void initialize_branch(Branch* kernel);

} // namespace circa

#endif
