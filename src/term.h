#ifndef CIRCA__TERM__INCLUDED
#define CIRCA__TERM__INCLUDED

#include "common_headers.h"

#include "ref_list.h"
#include "term_namespace.h"
#include "term_set.h"

namespace circa {

struct List;

typedef std::vector<std::string> ErrorList;

struct Term
{
    Branch* owningBranch;
    ReferenceList inputs;
    Term* function;
    TermSet users;

    // data type
    Term* type;

    // Our current value. This is meant to be transient.
    void* value;

    // Persisted value. Owned by us.
    Term* state;

    bool needsUpdate;

    ErrorList errors;

    int globalID;

    Term();
    ~Term();

    std::string toString();
    bool equals(Term* term);
    std::string findName();

    Term* field(std::string const& name) const;

    bool hasError() const;
    int numErrors() const;
    std::string const& getError(int index) const;
    void clearErrors();
    void pushError(std::string const& message);
    std::string getErrorMessage() const;

    int& asInt();
    float& asFloat();
    std::string& asString();
    bool& asBool();
    Term*& asRef();
    List& asList();

    void eval();
};

} // namespace circa

#endif
