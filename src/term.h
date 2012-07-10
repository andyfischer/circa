// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

#include "branch.h"
#include "dict.h"
#include "term_list.h"
#include "tagged_value.h"
#include "term_source_location.h"
#include "weak_ptrs.h"

namespace circa {

struct Term : public caTerm
{
    Value value;

    // Inherited from caTerm:
    // int id; // Globally unique ID. This is mainly used for debugging.

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

    // Our name binding.
    Name nameSymbol;
    std::string name;

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

    // Terms which are using this term as an input.
    TermList users;

    // Location in textual source code.
    TermSourceLocation sourceLoc;

    Term();
    ~Term();

    const char* nameStr();

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

    bool hasProperty(const char* name);
    void removeProperty(const char* name);

    int intProp(const char* name, int defaultValue);
    float floatProp(const char* name, float defaultValue);
    bool boolProp(const char* name, bool defaultValue);
    std::string stringProp(const char* name, const char* defaultValue);

    void setProp(const char* name, caValue* value);
    void setIntProp(const char* name, int i);
    void setFloatProp(const char* name, float f);
    void setBoolProp(const char* name, bool b);
    void setStringProp(const char* name, std::string const& s);
};

// Allocate a new Term object.
Term* alloc_term();
void dealloc_term(Term*);

// Fetches a term property, creating it if it doesn't exist.
caValue* term_insert_property(Term* term, const char* name);

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
