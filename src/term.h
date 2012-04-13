// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

#include "branch.h"
#include "dict.h"
#include "term_list.h"
#include "tagged_value.h"
#include "term_source_location.h"
#include "weak_ptrs.h"

namespace circa {

const int TERM_FLAG_LAZY = 0x1;

struct Term : public caTerm
{
    Value value;

    // Fields inherited from caValue:
    //   caValue::Data value_data
    //   Type* value_type

    // A WeakPtr to this object, this is lazily initialized.
    WeakPtr weakPtr;

    // A Type that statically describes our type.
    Type* type;

    struct Input {
        Term* term;
        Dict properties;

        Input() : term(NULL) {}
        Input(Term* t) : term(t) {}
    };

    typedef std::vector<Input> InputList;

    // Input terms
    InputList inputs;

    // Our function: the thing that takes our inputs and produces a value.
    Term* function;

    Name nameSymbol;

    // Our name binding.
#ifndef TERM_HAS_SYMBOL
    std::string name;
#endif

    // A name which is unique across this branch.
    struct UniqueName
    {
        std::string name;
        std::string base;
        int ordinal;
        UniqueName() : ordinal(0) {}
    };

    UniqueName uniqueName;

    // The branch that owns this term. May be NULL.
    Branch* owningBranch;

    // The index that this term currently holds inside owningBranch.
    int index;

    // Code which is nested inside this term. This object is created on-demand.
    Branch* nestedContents;

    // Dynamic properties
    Dict properties;

    int flags;

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

    // Shorthand for nested_contents()
    Branch* contents();

    // Shorthand for nested_contents()[index]
    Term* contents(int index);

    // Shorthand for nested_contents()[name]
    Term* contents(const char* name);

    std::string toString();

    bool hasProperty(std::string const& name);
    caValue* addProperty(std::string const& name, Term* type);
    void removeProperty(std::string const& name);

    int intProp(std::string const& name);
    float floatProp(std::string const& name);
    bool boolProp(std::string const& name);
    std::string const& stringProp(std::string const& name);

    void setProp(const char* name, caValue* value);
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

// Fetches a term property.
caValue* term_get_property(Term* term, const char* name);

// Assigns a property with the given name and value. The argument 'value' is
// consumed.
void term_set_property(Term* term, const char* name, caValue* value);

// Removes a term property.
void term_remove_property(Term* term, const char* name);

// Removes the property with the given name from 'source', and assigns that
// property to 'dest'. Has no effect if 'source' does not have the property.
void term_move_property(Term* source, Term* dest, const char* propName);

caValue* term_value(Term* term);
bool is_type(Term* term);
bool is_function(Term* term);
Function* as_function(Term* term);
Type* as_type(Term* term);

} // namespace circa
