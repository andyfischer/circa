#ifndef CIRCA__TERM__INCLUDED
#define CIRCA__TERM__INCLUDED

#include "common_headers.h"
#include "term_list.h"
#include "term_set.h"

namespace circa {

typedef std::vector<std::string> ErrorList;

struct Term
{
    Branch* owningBranch;
    TermList inputs;
    Term* function;
    TermSet users;

    void* value;
    Term* type;

    Term* state;

    bool needsUpdate;

    ErrorList errors;

    int globalID;

    Term();
    ~Term();

    Type* getType() const;
    std::string toString();
    std::string findName();

    int numErrors() const;
    std::string const& getError(int index);
};

TermList* as_list(Term* term);

void initialize_term(Branch* kernel);

} // namespace circa

#endif
