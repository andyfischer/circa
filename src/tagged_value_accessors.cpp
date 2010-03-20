// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "builtins.h"
#include "tagged_value.h"
#include "tagged_value_accessors.h"
#include "type.h"

namespace circa {

void assign_value(TaggedValue* source, TaggedValue* dest)
{
    if (source->value_type == NULL)
        throw std::runtime_error("In assign_value, source.value_type is NULL");
    if (dest->value_type == NULL)
        throw std::runtime_error("In assign_value, dest.value_type is NULL");

    // If dest type is 'any' then just change it now. This should be removed.
    if (dest->value_type == &as_type(ANY_TYPE))
        change_type(dest, source->value_type);

    // Check if they have different types. If so, try to cast.
    if (dest->value_type != source->value_type) {
        Type::Cast cast_func = dest->value_type->cast;
        if (cast_func == NULL) {
            throw std::runtime_error("No cast function for value_type "
                + dest->value_type->name + " (tried to assign value of value_type "
                + source->value_type->name + ")");
        }

        cast_func(dest->value_type, source, dest);
        return;
    }

    // Check if the type defines an assign function.
    Type::Assign assign = NULL;
    
    if (dest->value_type != NULL)
        assign = dest->value_type->assign;

    if (assign != NULL) {
        assign(source, dest);
    } else {
        // Otherwise, default behavior is shallow assign
        dest->value_data = source->value_data;
    }
}

bool cast_possible(Type* type, TaggedValue* value)
{
    Type::CastPossible castPossible = type->castPossible;

    if (castPossible != NULL)
        return castPossible(type, value);

    // Default behavior, only allow if types are exactly the same.
    return type == value->value_type;
}

void cast(Type* type, TaggedValue* source, TaggedValue* dest)
{
    if (type->cast == NULL) {
        // If types are equal and there's no cast() function, just do a copy.
        if (source->value_type == type) {
            copy(source, dest);
            return;
        }

        throw std::runtime_error("No cast function on type " + type->name);
    }

    type->cast(type, source, dest);
}

void copy(TaggedValue* source, TaggedValue* dest)
{
    change_type(dest, source->value_type);
    assign_value(source, dest);
}

void copy_newstyle(TaggedValue* source, TaggedValue* dest)
{
    change_type(dest, source->value_type);
    Type::Copy copy = source->value_type->copy;
    assert(copy != NULL);
    copy(source, dest);
}

void swap(TaggedValue* left, TaggedValue* right)
{
    Type* temp_type = left->value_type;
    TaggedValueData temp_data = left->value_data;
    left->value_type = right->value_type;
    left->value_data = right->value_data;
    right->value_type = temp_type;
    right->value_data = temp_data;
}

std::string to_string(TaggedValue* value)
{
    Type::ToString toString = value->value_type->toString;
    if (toString == NULL) {
        std::stringstream out;
        out << "<" << value->value_type->name << " " << value->value_data.ptr << ">";
        return out.str();
    }

    return toString(value);
}

void begin_modify(TaggedValue* value)
{
    Type::BeginModify beginModify = value->value_type->beginModify;
    if (beginModify != NULL)
        beginModify(value);

    // Default behavior: no-op.
}

void change_type(TaggedValue* v, Type* type)
{
    if (v->value_type == type)
        return;

    if (v->value_type != NULL) {
        Type::Release release = v->value_type->release;
        if (release != NULL)
            release(v);
    }

    v->value_type = type;
    v->value_data.ptr = 0;

    if (type != NULL) {
        Type::Initialize initialize = type->initialize;
        if (initialize != NULL)
            initialize(type, v);
    }
}

bool equals(TaggedValue* lhs, TaggedValue* rhs)
{
    assert(lhs->value_type != NULL);

    Type::Equals equals = lhs->value_type->equals;

    if (equals != NULL)
        return equals(lhs, rhs);

    // Default behavior for different types: return false
    if (lhs->value_type != rhs->value_type)
        return false;

    // Default behavor for same types: shallow comparison
    return lhs->value_data.asint == rhs->value_data.asint;
}

void make_int(TaggedValue* value, int i)
{
    change_type(value, INT_T);
    set_int(value, i);
}

void make_float(TaggedValue* value, float f)
{
    change_type(value, FLOAT_T);
    set_float(value, f);
}

void make_string(TaggedValue* value, const char* s)
{
    change_type(value, STRING_T);
    set_str(value, s);
}

void make_bool(TaggedValue* value, bool b)
{
    change_type(value, BOOL_T);
    set_bool(value, b);
}

void make_ref(TaggedValue* value, Term* t)
{
    change_type(value, &as_type(REF_TYPE));
    set_ref(value, t);
}

void make_type(TaggedValue* value, Type* type)
{
    value->value_type = (Type*) TYPE_TYPE->value_data.ptr;
    value->value_data.ptr = type;
}

void make_null(TaggedValue* value)
{
    change_type(value, NULL_T);
}

void set_int(TaggedValue* value, int i)
{
    assert(is_int(value));
    value->value_data.asint = i;
}

void set_float(TaggedValue* value, float f)
{
    assert(is_float(value));
    value->value_data.asfloat = f;
}

void set_bool(TaggedValue* value, bool b)
{
    assert(is_bool(value));
    value->value_data.asbool = b;
}

void set_str(TaggedValue* value, const char* s)
{
    assert(is_string(value));
    *((std::string*) value->value_data.ptr) = s;
}

void set_str(TaggedValue* value, std::string const& s)
{
    set_str(value, s.c_str());
}

void set_ref(TaggedValue* value, Term* t)
{
    assert(is_ref(value));
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

int as_int(TaggedValue* value)
{
    assert(is_int(value));
    return value->value_data.asint;
}

float as_float(TaggedValue* value)
{
    assert(is_float(value));
    return value->value_data.asfloat;
}

bool as_bool(TaggedValue* value)
{
    assert(is_bool(value));
    return value->value_data.asbool;
}

Ref& as_ref(TaggedValue* value)
{
    assert(is_ref(value));
    return *((Ref*) value->value_data.ptr);
}

Type& as_type(TaggedValue* value)
{
    assert(is_type(value));
    return *((Type*) value->value_data.ptr);
}

std::string const& as_string(TaggedValue* value)
{
    assert(is_string(value));
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

int to_int(TaggedValue* value)
{
    if (value->value_type == INT_TYPE->value_data.ptr)
        return as_int(value);
    else if (value->value_type == FLOAT_TYPE->value_data.ptr)
        return (int) as_float(value);
    else
        throw std::runtime_error("In to_int, type is not an int or float");
}

bool is_int(TaggedValue* value)
{
    return INT_TYPE != NULL
        && value->value_type == (Type*) INT_TYPE->value_data.ptr;
}

bool is_float(TaggedValue* value)
{
    return FLOAT_TYPE != NULL
        && value->value_type == (Type*) FLOAT_TYPE->value_data.ptr;
}

bool is_bool(TaggedValue* value)
{
    return BOOL_TYPE != NULL
        && value->value_type == (Type*) BOOL_TYPE->value_data.ptr;
}

bool is_string(TaggedValue* value)
{
    return STRING_TYPE != NULL
        && value->value_type == (Type*) STRING_TYPE->value_data.ptr;
}

bool is_ref(TaggedValue* value)
{
    return REF_TYPE != NULL
        && value->value_type == (Type*) REF_TYPE->value_data.ptr;
}

bool is_value_branch(TaggedValue* value)
{
    return BRANCH_TYPE != NULL
        && value->value_type == (Type*) BRANCH_TYPE->value_data.ptr;
}

bool is_type(TaggedValue* value)
{
    return TYPE_TYPE != NULL
        && value->value_type == (Type*) TYPE_TYPE->value_data.ptr;
}

bool is_value_of_type(TaggedValue* value, Type* type)
{
    return value->value_type == type;
}

bool is_null(TaggedValue* value)
{
    return value->value_type == NULL_T;
}

} // namespace circa
