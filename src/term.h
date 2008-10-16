#ifndef CIRCA_TERM_INCLUDED
#define CIRCA_TERM_INCLUDED

#include "common_headers.h"

#include "ref_list.h"
#include "ref_set.h"
#include "dictionary.h"

namespace circa {

struct List;

typedef std::vector<std::string> ErrorList;

void assert_good(Term* term);

struct Term
{
    Branch* owningBranch;
    ReferenceList inputs;
    Term* function;

    // data type
    Term* type;

    // Our current value. This is meant to be transient.
    void* value;

    // If true, recycle_value is allowed to steal our value
    bool stealingOk;

    // Persisted value. Owned by us.
    Term* state;

    // Our name. We might have other aliases, according to the
    // owning branch.
    std::string name;

    Dictionary properties;

    bool needsUpdate;

    ErrorList errors;

    int globalID;

    Term();
    ~Term();

    std::string toString();
    bool equals(Term* term);
    std::string findName();

    Term* field(std::string const& name);

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
