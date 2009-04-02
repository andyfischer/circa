// Copyright 2008 Paul Hodge

#ifndef CIRCA_TERM_INCLUDED
#define CIRCA_TERM_INCLUDED

#include "common_headers.h"

#include "branch.h"
#include "references.h"
#include "ref_list.h"

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

    // Our name binding.
    std::string name;

    // The branch that owns this term.
    Branch* owningBranch;

    // If true, assign_value is allowed to steal our value.
    bool stealingOk;

    // True if this term's value is out-of-date
    bool needsUpdate;

    // A globally unique ID
    unsigned int globalID;

    // Dynamic properties
    Branch properties;

    // Reference count
    int refCount;

    Term();
    ~Term();

    Term* input(int index) const;
    int numInputs() const;

    std::string toString();

    bool hasValue();

    // Returns the named property
    Term* property(std::string const& name) const;

    bool hasProperty(std::string const& name) const;
    Term* addProperty(std::string const& name, Term* type);
    void removeProperty(std::string const& name);

    bool& boolProperty(std::string const& name);
    int& intProperty(std::string const& name);
    float& floatProperty(std::string const& name);
    std::string& stringProperty(std::string const& name);

    bool boolPropertyOptional(std::string const& name, bool defaultValue);
    float floatPropertyOptional(std::string const& name, float defaultValue);
    int intPropertyOptional(std::string const& name, int defaultValue);
    std::string stringPropertyOptional(std::string const& name, std::string const& defaultValue);

    // Accessors for specific properties

    bool hasError() const;
    void clearError();
    void pushError(std::string const& message);
    std::string getErrorMessage() const;

    // Convenience accessors
    int& asInt();
    float& asFloat();
    std::string& asString();
    bool& asBool();
    Ref& asRef();
    Branch& asBranch();

    // Accessors for compound types
    Term* field(int index);
    Term* field(std::string const& fieldName);
};

} // namespace circa

#endif
