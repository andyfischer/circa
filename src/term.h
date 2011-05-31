// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

#include "branch.h"
#include "term_list.h"
#include "name_list.h"
#include "tagged_value.h"
#include "term_source_location.h"
#include "types/dict.h"

namespace circa {

struct Term : TaggedValue
{
    struct UniqueName
    {
        std::string name;
        std::string base;
        int ordinal;
        UniqueName() : ordinal(0) {}
    };

    struct Input {
        Term* term;
        int outputIndex;
        Dict properties;

        Input() : outputIndex(0) {}
        Input(Term* t) : term(t), outputIndex(0) {}
        Input(Term* t, int i) : term(t), outputIndex(i) {}
    };

    typedef std::vector<Input> InputList;

    // Fields inherited from TaggedValue:
    //   TaggedValue::Data value_data
    //   Type* value_type

    // A Type term that describes our data type
    Term* type;

    // Input terms
    InputList inputs;

    // Our function: the thing that takes our inputs and produces a value.
    Term* function;

    // A function pointer to the C function which does the actual work. This is
    // derived from 'function'. Used during interpreted evaluation.
    EvaluateFunc evaluateFunc;

    // Our name binding.
    std::string name;

    // Names for extra outputs
    NameList additionalOutputNames;

    // A name which is unique across this branch.
    UniqueName uniqueName;

    // The branch that owns this term. May be NULL
    Branch* owningBranch;

    // The index that this term currently holds inside owningBranch
    int index;

    // The location of this term's output in the locals list. If the term has multiple
    // outputs then this is the first index.
    int localsIndex;

    // Number of outputs.
    int outputCount;

    // Code which is nested inside this term. Usually this is empty.
    Branch nestedContents;

    // A globally unique ID
    unsigned int globalID;

    // Dynamic properties
    Dict properties;

    // Terms which are using this term as an input.
    TermList users;

    // Location in textual source code.
    TermSourceLocation sourceLoc;

    Term();
    ~Term();

    Term* input(int index) const;
    Input* inputInfo(int index);
    int numInputs() const;

    void inputsToList(TermList& out) const;

    // In this context, the 'dependencies' include the function term and all input
    // terms. So, this is the same as the input list, with the function inserted
    // at element 0. This is more convenient in some situations.
    Term* dependency(int index) const;
    int numDependencies() const;
    void setDependency(int index, Term* term);

    int numOutputs() const;

    const char* getName(int index) const;
    int nameCount() const;

    std::string toString();

    // Returns the named property
    TaggedValue* property(std::string const& name);

    bool hasProperty(std::string const& name);
    TaggedValue* addProperty(std::string const& name, Term* type);
    void removeProperty(std::string const& name);

    int intProp(std::string const& name);
    float floatProp(std::string const& name);
    bool boolProp(std::string const& name);
    std::string const& stringProp(std::string const& name);

    void setIntProp(std::string const& name, int i);
    void setFloatProp(std::string const& name, float f);
    void setBoolProp(std::string const& name, bool b);
    void setStringProp(std::string const& name, std::string const& s);

    int intPropOptional(std::string const& name, int defaultValue);
    float floatPropOptional(std::string const& name, float defaultValue);
    bool boolPropOptional(std::string const& name, bool defaultValue);
    std::string stringPropOptional(std::string const& name, std::string const& defaultValue);
};

// Allocate a new Term object.
Term* alloc_term();
void dealloc_term(Term*);

} // namespace circa
