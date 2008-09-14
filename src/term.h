#ifndef CIRCA__TERM__INCLUDED
#define CIRCA__TERM__INCLUDED

#include "common_headers.h"

#include "term_list.h"
#include "term_namespace.h"
#include "term_set.h"

namespace circa {

typedef std::vector<std::string> ErrorList;

struct Term
{
    Branch* owningBranch;
    TermList inputs;
    Term* function;
    TermSet users;
    Term* type;

    // 'localBranch' includes the 'state' term and any terms that are
    // part of 'fields'. This field starts out as NULL, and a branch is
    // created when needed.
    Branch* localBranch;

    // Our raw value. This is meant to be transient. For example, if we are a pure
    // function, the executor is allowed to reevaluate us at any time, and is thus
    // allowed to (temporarily) throw out our value.
    void* value;

    // Considered part of our transient value, allows for compound types.
    TermNamespace fields;

    // Persisted value
    Term* state;

    bool needsUpdate;

    ErrorList errors;

    int globalID;

    Term();
    ~Term();

    std::string toString();
    bool equals(Term* term);
    std::string findName();

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
