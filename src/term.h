// Copyright 2008 Paul Hodge

#ifndef CIRCA_TERM_INCLUDED
#define CIRCA_TERM_INCLUDED

#include "common_headers.h"

#include "dictionary.h"
#include "ref.h"
#include "ref_list.h"
#include "term_syntax_hints.h"

namespace circa {

struct List;

struct Term
{
    // Our current value. In some situations, when someone refers to this term, they
    // may be referring to this value. May be NULL. The type of this value is described
    // by 'type'.
    void* value;

    // A Type term that describes our data type
    Ref type;

    // Input terms
    RefList inputs;

    // Our function: the thing that takes our inputs (and possibly state), and produces a value.
    Ref function;

    // Persisted internal value. Owned by us.
    Ref state;

    // The branch that owns this term.
    Branch* owningBranch;

    // Whether this term owns our value. Usually this is true.
    bool ownsValue;

    // If true, recycle_value is allowed to steal our value.
    bool stealingOk;

    // Our name binding.
    std::string name;

    // True if this term's value is out-of-date
    bool needsUpdate;

    // A globally unique ID
    unsigned int globalID;

    // Syntax hints, used for source code reproduction
    TermSyntaxHints syntaxHints;

    // Dynamic properties
    Dictionary properties;

    // 'users' are terms which are using us as an input or as a function.
    RefList users;

    // Objects which are pointing to us
    std::vector<Ref*> refs;

    Term();
    ~Term();

    Term* input(int index) const;
    int numInputs() const;

    std::string toString();

    // Returns the named property
    Term* property(std::string const& name) const;

    bool hasProperty(std::string const& name) const;
    Term* addProperty(std::string const& name, Term* type);
    void removeProperty(std::string const& name);

    // Property accessors

    bool hasError() const;
    void clearError();
    void pushError(std::string const& message);
    std::string getErrorMessage() const;

    bool isStateful() const;
    void setIsStateful(bool value);

    // Convenience accessors
    int& asInt();
    float& asFloat();
    std::string& asString();
    bool& asBool();
    Term*& asRef();
    List& asList();
    Branch& asBranch();

    // Accessors for compound types
    Term* field(int index);
    Term* field(std::string const& fieldName);
};

} // namespace circa

#endif
