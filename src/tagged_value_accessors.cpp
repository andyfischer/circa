// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "builtins.h"
#include "tagged_value.h"
#include "tagged_value_accessors.h"
#include "primitives.h"
#include "type.h"

namespace circa {

void assign_value(TaggedValue* source, TaggedValue* dest)
{
    // Temp for compatibility: if either type is NULL then just do shallow assign
    if (source->value_type == NULL || dest->value_type == NULL) {
        dest->value_data = source->value_data;
        return;
    }

    if (source->value_type == NULL)
        throw std::runtime_error("In assign_value, source.value_type is NULL");
    if (dest->value_type == NULL)
        throw std::runtime_error("In assign_value, dest.value_type is NULL");

    // Check if they have different types. If so, try to cast.
    if (dest->value_type != source->value_type) {
        Type::CastFunc cast = dest->value_type->cast;
        if (cast == NULL)
            throw std::runtime_error("No cast function for value_type "
                + dest->value_type->name + " (tried to assign value of value_type "
                + source->value_type->name + ")");

        cast(dest->value_type, source, dest);
        return;
    }

    // Check if the type defines an assign function.
    Type::AssignFunc2 assign = NULL;
    
    if (dest->value_type != NULL)
        assign = dest->value_type->assign2;

    if (assign != NULL) {
        assign(source, dest);
    } else {
        // Otherwise, default behavior is shallow assign
        dest->value_data = source->value_data;
    }
}

void change_type(TaggedValue* v, Type* type)
{
    if (v->value_type == type)
        return;

    if (v->value_type != NULL) {
        Type::DestroyFunc destroy = v->value_type->destroy;
        if (destroy != NULL)
            destroy(v->value_type, v);
    }

    v->value_type = type;
    v->value_data.ptr = 0;

    if (type != NULL) {
        Type::InitializeFunc initialize = type->initialize;
        if (initialize != NULL)
            initialize(type, v);
    }
}

bool equals(TaggedValue* lhs, TaggedValue* rhs)
{
    if (lhs->value_type != rhs->value_type)
        return false;

    assert(lhs->value_type != NULL);

    Type::EqualsFunc2 equals2 = lhs->value_type->equals2;

    if (equals2 != NULL)
        return equals2(lhs, rhs);

    // Default behavior: shallow-comparison
    return lhs->value_data.asint == rhs->value_data.asint;
}

void set_branch_value(TaggedValue* value, Branch* branch)
{
    value->value_type = (Type*) BRANCH_TYPE->value_data.ptr;
    value->value_data.ptr = branch;
}

void set_type_value(TaggedValue* value, Type* type)
{
    value->value_type = (Type*) TYPE_TYPE->value_data.ptr;
    value->value_data.ptr = type;
}

void set_int(TaggedValue* value, int i)
{
    value->value_type = (Type*) INT_TYPE->value_data.ptr;
    value->value_data.asint = i;
}

void set_float(TaggedValue* value, float f)
{
    value->value_type = (Type*) FLOAT_TYPE->value_data.ptr;
    value->value_data.asfloat = f;
}

void set_bool(TaggedValue* value, bool b)
{
    value->value_type = (Type*) BOOL_TYPE->value_data.ptr;
    value->value_data.asbool = b;
}

void set_str(TaggedValue* value, const char* s)
{
    if (value->value_type != STRING_TYPE->value_data.ptr) {
        value->value_type = (Type*) STRING_TYPE->value_data.ptr;
        value->value_data.ptr = new std::string();
    }
    *((std::string*) value->value_data.ptr) = s;
}

void set_str(TaggedValue* value, std::string const& s)
{
    set_str(value, s.c_str());
}

void set_ref(TaggedValue* value, Term* t)
{
    assert(is_value_ref(value));
    *((Ref*) value->value_data.ptr) = t;
}

void set_null(TaggedValue* value)
{
    change_type(value, NULL_T);
}

void set_pointer(TaggedValue* value, Type* type, void* p)
{
    value->value_type = type;
    value->value_data.ptr = p;
}

#if 0
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
#endif

int as_int(TaggedValue* value)
{
    assert(is_value_int(value));
    return value->value_data.asint;
}

float as_float(TaggedValue* value)
{
    assert(is_value_float(value));
    return value->value_data.asfloat;
}

bool as_bool(TaggedValue* value)
{
    assert(is_value_bool(value));
    return value->value_data.asbool;
}

Ref& as_ref(TaggedValue* value)
{
    assert(is_value_ref(value));
    return *((Ref*) value->value_data.ptr);
}

Type& as_type(TaggedValue* value)
{
    assert(is_value_type(value));
    return *((Type*) value->value_data.ptr);
}

std::string const& as_string(TaggedValue* value)
{
    assert(is_value_string(value));
    return *((std::string*) value->value_data.ptr);
}

void* get_pointer(TaggedValue* value)
{
    return value->value_data.ptr;
}

void set_pointer(TaggedValue* value, void* ptr)
{
    value->value_data.ptr = ptr;
}

Type* get_type_value(TaggedValue* value)
{
    //assert(value.value_type == &as_type(TYPE_TYPE));
    return (Type*) value->value_data.ptr;
}

Branch* get_branch_value(TaggedValue* value)
{
    //assert(value.value_type == &as_type(TYPE_TYPE));
    return (Branch*) value->value_data.ptr;
}

Branch& as_branch(TaggedValue* value)
{
    assert(is_branch(value));
    return *((Branch*) value->value_data.ptr);
}

const char* get_name_for_type(Type* type)
{
    if (type == NULL)
        return "<NULL>";
    else return type->name.c_str();
}

void* get_pointer(TaggedValue* value, Type* expectedType)
{
    if (value->value_type != expectedType) {
        std::stringstream strm;
        strm << "Type mismatch in get_pointer, expected " << get_name_for_type(expectedType);
        strm << ", but value has type " << get_name_for_type(value->value_type);
        throw std::runtime_error(strm.str());
    }

    return value->value_data.ptr;
}

float to_float(TaggedValue* value)
{
    if (value->value_type == INT_TYPE->value_data.ptr)
        return as_int(value);
    else if (value->value_type == FLOAT_TYPE->value_data.ptr)
        return as_float(value);
    else
        throw std::runtime_error("In to_float, type is not an int or float");
}

bool is_value_int(TaggedValue* value)
{
    return value->value_type == (Type*) INT_TYPE->value_data.ptr;
}

bool is_value_float(TaggedValue* value)
{
    return value->value_type == (Type*) FLOAT_TYPE->value_data.ptr;
}

bool is_value_bool(TaggedValue* value)
{
    return value->value_type == (Type*) BOOL_TYPE->value_data.ptr;
}

bool is_value_string(TaggedValue* value)
{
    return value->value_type == (Type*) STRING_TYPE->value_data.ptr;
}

bool is_value_ref(TaggedValue* value)
{
    return value->value_type == (Type*) REF_TYPE->value_data.ptr;
}

bool is_value_branch(TaggedValue* value)
{
    return value->value_type == (Type*) BRANCH_TYPE->value_data.ptr;
}

bool is_value_type(TaggedValue* value)
{
    return value->value_type == (Type*) TYPE_TYPE->value_data.ptr;
}

bool is_value_of_type(TaggedValue* value, Type* type)
{
    return value->value_type == type;
}

bool is_branch(TaggedValue* value)
{
    return value->value_type->initialize == branch_t::initialize;
}

bool is_null(TaggedValue* value)
{
    return value->value_type == NULL_T;
}

} // namespace circa
