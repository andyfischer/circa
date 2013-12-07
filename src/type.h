// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include <typeinfo>

#include "circa/circa.h"

#include "block.h"
#include "heap_debugging.h"
#include "object.h"
#include "term_map.h"
#include "term.h"
#include "term_namespace.h"

namespace circa {

extern Term* IMPLICIT_TYPES;

struct CastResult
{
    bool success;

    CastResult() : success(true) {}
};

struct Type
{
    typedef void (*Initialize)(Type* type, caValue* value);
    typedef void (*Copy)(Type* type, caValue* source, caValue* dest);
    typedef void (*Reset)(Type* type, caValue* value);
    typedef bool (*Equals)(caValue* lhs, caValue* rhs);

    // Attempts to cast 'value' to the given type. If the cast isn't possible, callee will
    // record the failure in the CastResult.
    //
    // If checkOnly is true, then callee should only record whether the cast is possible,
    // and not actually modify 'dest'.
    typedef void (*Cast)(CastResult* result, caValue* value, Type* type, bool checkOnly);

    typedef void (*StaticTypeQueryFunc)(Type* type, StaticTypeQuery* query);
    typedef std::string (*ToString)(caValue* value);
    typedef void (*FormatSource)(caValue*, Term* term);
    typedef void (*Touch)(caValue* value);
    typedef caValue* (*GetIndex)(caValue* value, int index);
    typedef void (*SetIndex)(caValue* value, int index, caValue* element);
    typedef int (*NumElements)(caValue* value);
    typedef bool (*CheckInvariants)(Term* term, std::string* output);
    typedef void (*RemapPointers)(Term* term, TermMap const& map);
    typedef int (*HashFunc)(caValue* value);
    typedef void (*VisitHeapCallback)(caValue* value, caValue* relativeIdentifier,
            caValue* context);
    typedef void (*VisitHeap)(Type* type, caValue* value,
            VisitHeapCallback callback, caValue* context);

    typedef int (*ChecksumFunc)(caValue* value);

    // CircaObject header, must be the first field.
    CircaObject header;

    int id;

    Value name;

    // Enum describing the data structure used to store this type's value.
    Symbol storageType;

    Term* declaringTerm;

    // Functions
    Initialize initialize;
    ReleaseFunc release;
    Copy copy;
    Reset reset;
    Equals equals;
    Cast cast;
    StaticTypeQueryFunc staticTypeQuery;
    ToString toString;
    FormatSource formatSource;
    Touch touch;
    GetIndex getIndex;
    SetIndex setIndex;
    NumElements numElements;
    CheckInvariants checkInvariants;
    RemapPointers remapPointers;
    HashFunc hashFunc;
    VisitHeap visitHeap;
    ChecksumFunc checksum;

    // Parent type, may be null.
    Type* parent;

    Value properties;
    
    // Type parameters
    Value parameter;
    int objectSize;

    bool inUse;

    const char* nameStr();

private:
    // Disallow C++ stuff.
    Type() { internal_error(""); }
    ~Type() { internal_error(""); }
    Type(Type const&) { internal_error(""); }
    Type& operator=(Type const&) { internal_error(""); return *this; }
};


struct StaticTypeQuery
{
    enum Result {
        NULL_RESULT,
        SUCCEED,
        FAIL,
        UNABLE_TO_DETERMINE
    };

    // The type that we are checking against.
    Type* type;

    // The term that is being checked. The query will check to see if all the
    // outputs from this subject conform to the type. This is sometimes NULL
    // (such as for compound type checks)
    Term* subject;

    // The type of the subject. This is always available even if 'subject' is NULL.
    Type* subjectType;
    
    // Outputs
    Result result;


    StaticTypeQuery() : type(NULL), subject(NULL), subjectType(NULL),
        result(NULL_RESULT) {}

    void fail() { result = FAIL; }
    void succeed() { result = SUCCEED; }
    void unableToDetermine() { result = UNABLE_TO_DETERMINE; }
};

namespace type_t {

    void initialize(Type* type, caValue* value);
    void copy(Type*, caValue* source, caValue* dest);
    std::string to_string(Term *caller);
    void formatSource(caValue* source, Term* term);
    void remap_pointers(Term *term, TermMap const& map);
    void setup_type(Type* type);
} // namespace type_t

// Create a Type that isn't finished being built. Only appropriate during bootstrapping.
Type* create_type_unconstructed();
void type_finish_construction(Type* type);

// Create a Type object.
Type* create_type();
void delete_type(Type* type);
void predelete_type(Type* type);

void type_start_at_zero_refs(Type* type);

void type_incref(Type* type);
void type_decref(Type* type);
bool type_is_root(Type* type);
void type_set_root(Type* type);

Type* unbox_type(Term* type);
Type* unbox_type(caValue* val);

Type* get_output_type(Term* term, int outputIndex);
Type* get_output_type(Term* term);
Type* get_type_of_input(Term* term, int inputIndex);
caValue* get_type_property(Type* type, const char* name);
caValue* type_property_insert(Type* type, const char* name);
void set_type_property(Type* type, const char* name, caValue* value);

Block* type_declaration_block(Type* type);

StaticTypeQuery::Result run_static_type_query(Type* type, Type* subjectType);
StaticTypeQuery::Result run_static_type_query(Type* type, Term* term);
bool term_output_always_satisfies_type(Term* term, Type* type);
bool term_output_never_satisfies_type(Term* term, Type* type);

void reset_type(Type* type);
void clear_type_contents(Type* type);

void initialize_simple_pointer_type(Type* type);

Term* find_method(Block* block, Type* type, caValue* name);

// Change the type value for an existing type. 'term' should be a value of
// type Type.
void install_type(Term* term, Type* type);

Type* find_type(Block* block, const char* name);

void set_type_list(caValue* value, Type* type1);
void set_type_list(caValue* value, Type* type1, Type* type2);
void set_type_list(caValue* value, Type* type1, Type* type2, Type* type3);

Term* type_decl_append_field(Block* declaration, const char* fieldName, Term* fieldType);

void setup_interface_type(Type* type);
void type_install_functions(Block* block);

} // namespace circa
