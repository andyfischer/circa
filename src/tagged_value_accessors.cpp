// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "builtins.h"
#include "tagged_value.h"
#include "tagged_value_accessors.h"
#include "primitives.h"
#include "type.h"

namespace circa {

void assign_value(TaggedValue& source, TaggedValue& dest)
{
    // Temp for compatibility: if either type is NULL then just do shallow assign
    if (source.type == NULL || dest.type == NULL) {
        dest.data = source.data;
        return;
    }

    if (source.type == NULL)
        throw std::runtime_error("In assign_value, source.type is NULL");
    if (dest.type == NULL)
        throw std::runtime_error("In assign_value, dest.type is NULL");

    // Check if they have different types. If so, try to cast.
    if (dest.type != source.type) {
        Type::CastFunc cast = dest.type->cast;
        if (cast == NULL)
            throw std::runtime_error("No cast function for type "
                + dest.type->name + " (tried to assign value of type "
                + source.type->name + ")");

        cast(dest.type, &source, &dest);
        return;
    }

    // Check if the type defines an assign function.
    Type::AssignFunc2 assign = NULL;
    
    if (dest.type != NULL)
        assign = dest.type->assign2;

    if (assign != NULL) {
        assign(&source, &dest);
    } else {
        // Otherwise, default behavior is shallow assign
        dest.data = source.data;
    }
}

void change_type(TaggedValue& v, Type* type)
{
    if (v.type == type)
        return;

    if (v.type != NULL) {
        Type::DestroyFunc destroy = v.type->destroy;
        if (destroy != NULL)
            destroy(v.type, &v);
    }

    v.type = type;
    v.data.ptr = 0;

    if (type != NULL) {
        Type::InitializeFunc initialize = type->initialize;
        if (initialize != NULL)
            initialize(type, &v);
    }
}

bool equals(TaggedValue& lhs, TaggedValue& rhs)
{
    if (lhs.type != rhs.type)
        return false;

    assert(lhs.type != NULL);

    Type::EqualsFunc2 equals2 = lhs.type->equals2;

    if (equals2 != NULL)
        return equals2(&lhs, &rhs);

    // Default behavior: shallow-comparison
    return lhs.data.asint == rhs.data.asint;
}

void set_branch_value(TaggedValue& value, Branch* branch)
{
    value.type = (Type*) BRANCH_TYPE->value.data.ptr;
    value.data.ptr = branch;
}

void set_type_value(TaggedValue& value, Type* type)
{
    value.type = (Type*) TYPE_TYPE->value.data.ptr;
    value.data.ptr = type;
}

void set_int(TaggedValue& value, int i)
{
    value.type = (Type*) INT_TYPE->value.data.ptr;
    value.data.asint = i;
}

void set_float(TaggedValue& value, float f)
{
    value.type = (Type*) FLOAT_TYPE->value.data.ptr;
    value.data.asfloat = f;
}

void set_bool(TaggedValue& value, bool b)
{
    value.type = (Type*) BOOL_TYPE->value.data.ptr;
    value.data.asbool = b;
}

void set_str(TaggedValue& value, const char* s)
{
    if (value.type != STRING_TYPE->value.data.ptr) {
        value.type = (Type*) STRING_TYPE->value.data.ptr;
        value.data.ptr = new std::string();
    }
    *((std::string*) value.data.ptr) = s;
}

void set_str(TaggedValue& value, std::string const& s)
{
    set_str(value, s.c_str());
}

void set_ref(TaggedValue& value, Term* t)
{
    assert(is_value_ref(value));
    *((Ref*) value.data.ptr) = t;
}

void set_null(TaggedValue& value)
{
    change_type(value, NULL_T);
}

void set_pointer(TaggedValue& value, Type* type, void* p)
{
    value.type = type;
    value.data.ptr = p;
}

void set_branch_value(Term* term, Branch* branch) { set_branch_value(term->value, branch); }
void set_type_value(Term* term, Type* type) { set_type_value(term->value, type); }
void set_int(Term* term, int i) { set_int(term->value, i); }
void set_float(Term* term, float f) { set_float(term->value, f); }
void set_bool(Term* term, bool b) { set_bool(term->value, b); }
void set_str(Term* term, std::string const& s) { set_str(term->value, s); }
void set_str(Term* term, const char* s) { set_str(term->value, s); }
void set_ref(Term* term, Term* t) { set_ref(term->value, t); }
void set_null(Term* term) { set_null(term->value); }

TaggedValue tag_int(int v)
{
    TaggedValue result;
    change_type(result, (Type*) INT_TYPE->value.data.ptr);
    set_int(result, v);
    return result;
}

TaggedValue tag_float(float f)
{
    TaggedValue result;
    change_type(result, (Type*) FLOAT_TYPE->value.data.ptr);
    set_float(result, f);
    return result;
}
TaggedValue tag_bool(bool b)
{
    TaggedValue result;
    set_bool(result, b);
    return result;
}
TaggedValue tag_str(const char* s)
{
    TaggedValue result;
    set_str(result, s);
    return result;
}
TaggedValue tag_str(std::string const& s)
{
    TaggedValue result;
    set_str(result, s);
    return result;
}
TaggedValue tag_null()
{
    TaggedValue result;
    set_null(result);
    return result;
}
TaggedValue tag_pointer(Type* type, void* value)
{
    TaggedValue result;
    set_pointer(result, type, value);
    return result;
}

int as_int(TaggedValue const& value)
{
    assert(is_value_int(value));
    return value.data.asint;
}

float as_float(TaggedValue const& value)
{
    assert(is_value_float(value));
    return value.data.asfloat;
}

bool as_bool(TaggedValue const& value)
{
    assert(is_value_bool(value));
    return value.data.asbool;
}

Ref& as_ref(TaggedValue const& value)
{
    assert(is_value_ref(value));
    return *((Ref*) value.data.ptr);
}

Type& as_type(TaggedValue const& value)
{
    assert(is_value_type(value));
    return *((Type*) value.data.ptr);
}

std::string const& as_string(TaggedValue const& value)
{
    assert(is_value_string(value));
    return *((std::string*) value.data.ptr);
}

Type* get_type_value(TaggedValue const& value)
{
    //assert(value.type == &as_type(TYPE_TYPE));
    return (Type*) value.data.ptr;
}

Branch* get_branch_value(TaggedValue const& value)
{
    //assert(value.type == &as_type(TYPE_TYPE));
    return (Branch*) value.data.ptr;
}

const char* get_name_for_type(Type* type)
{
    if (type == NULL)
        return "<NULL>";
    else return type->name.c_str();
}

void* get_pointer(TaggedValue const& value, Type* expectedType)
{
    if (value.type != expectedType) {
        std::stringstream strm;
        strm << "Type mismatch in get_pointer, expected " << get_name_for_type(expectedType);
        strm << ", but value has type " << get_name_for_type(value.type);
        throw std::runtime_error(strm.str());
    }

    return value.data.ptr;
}

int as_int(Term* t) { return as_int(t->value); }
float as_float(Term* t) { return as_float(t->value); }
std::string const& as_string(Term* t) { return as_string(t->value); }
bool as_bool(Term* t) { return as_bool(t->value); }
Ref& as_ref(Term* t) { return as_ref(t->value); }
void* get_pointer(Term* term, Type* expectedType) { return get_pointer(term->value, expectedType); }

float to_float(TaggedValue const& value)
{
    if (value.type == INT_TYPE->value.data.ptr)
        return as_int(value);
    else if (value.type == FLOAT_TYPE->value.data.ptr)
        return as_float(value);
    else
        throw std::runtime_error("In to_float, type is not an int or float");
}

bool is_value_int(TaggedValue const& value)
{
    return value.type == (Type*) INT_TYPE->value.data.ptr;
}

bool is_value_float(TaggedValue const& value)
{
    return value.type == (Type*) FLOAT_TYPE->value.data.ptr;
}

bool is_value_bool(TaggedValue const& value)
{
    return value.type == (Type*) BOOL_TYPE->value.data.ptr;
}

bool is_value_string(TaggedValue const& value)
{
    return value.type == (Type*) STRING_TYPE->value.data.ptr;
}

bool is_value_ref(TaggedValue const& value)
{
    return value.type == (Type*) REF_TYPE->value.data.ptr;
}

bool is_value_branch(TaggedValue const& value)
{
    return value.type == (Type*) BRANCH_TYPE->value.data.ptr;
}

bool is_value_type(TaggedValue const& value)
{
    return value.type == (Type*) TYPE_TYPE->value.data.ptr;
}

bool is_value_of_type(TaggedValue const& value, Type* type)
{
    return value.type == type;
}

bool is_null(TaggedValue const& value)
{
    return value.type == NULL_T;
}

bool is_value_int(Term* t) { return is_value_int(t->value); }
bool is_value_float(Term* t) { return is_value_float(t->value); }
bool is_value_bool(Term* t) { return is_value_bool(t->value); }
bool is_value_string(Term* t) { return is_value_string(t->value); }
bool is_value_ref(Term* t) { return is_value_ref(t->value); }
bool is_value_branch(Term* t) { return is_value_branch(t->value); }
bool is_value_of_type(Term* t, Type* type) { return is_value_of_type(t->value, type); }

} // namespace circa
