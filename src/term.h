#ifndef CIRCA__TERM__INCLUDED
#define CIRCA__TERM__INCLUDED

#include "common_headers.h"

#include "ref_list.h"
#include "term_namespace.h"
#include "term_set.h"

namespace circa {

typedef std::vector<std::string> ErrorList;

struct Term
{
    Branch* owningBranch;
    ReferenceList inputs;
    Term* function;
    TermSet users;
    Term* type;

    // Our raw value. This is meant to be transient. For example, if we are a pure
    // function, the executor is allowed to reevaluate us at any time, and is thus
    // allowed to (temporarily) throw out our value.
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

    void eval();
};

} // namespace circa

#endif
