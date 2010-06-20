// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "errors.h"
#include "tagged_value.h"
#include "type.h"

namespace circa {

TaggedValue::TaggedValue()
{
    init();
}

void
TaggedValue::init()
{
    value_type = NULL_T;
    value_data.ptr = 0;
}

TaggedValue::~TaggedValue()
{
    // Deallocate this value
    change_type(this, NULL_T);
}

TaggedValue::TaggedValue(TaggedValue const& original)
{
    value_type = NULL_T;
    value_data.ptr = 0;

    Term* source = (Term*) &original;
    copy(source, this);
}

TaggedValue&
TaggedValue::operator=(TaggedValue const& rhs)
{
    Term* source = (Term*) &rhs;
    copy(source, this);
    return *this;
}

TaggedValue::TaggedValue(Type* type)
{
    init();
    change_type(this, type);
}

void TaggedValue::reset()
{
    circa::reset(this);
}

std::string
TaggedValue::toString()
{
    return to_string(this);
}

TaggedValue*
TaggedValue::getIndex(int index)
{
    return get_index(this, index);
}

TaggedValue*
TaggedValue::getField(const char* fieldName)
{
    return get_field(this, fieldName);
}

TaggedValue*
TaggedValue::getField(std::string const& fieldName)
{
    return get_field(this, fieldName.c_str());
}

int
TaggedValue::numElements()
{
    return num_elements(this);
}

bool
TaggedValue::equals(TaggedValue* rhs)
{
    return circa::equals(this, rhs);
}

int TaggedValue::asInt()
{
    return as_int(this);
}

float TaggedValue::asFloat()
{
    return as_float(this);
}

float TaggedValue::toFloat()
{
    return to_float(this);
}

std::string const& TaggedValue::asString()
{
    return as_string(this);
}

bool TaggedValue::asBool()
{
    return as_bool(this);
}

Ref& TaggedValue::asRef()
{
    return as_ref(this);
}

Branch& TaggedValue::asBranch()
{
    return as_branch(this);
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

        std::string msg = "No cast function on type " + type->name;
        internal_error(msg.c_str());
    }

    type->cast(type, source, dest);
}

void cast(TaggedValue* source, TaggedValue* dest)
{
    return cast(dest->value_type, source, dest);
}

void copy(TaggedValue* source, TaggedValue* dest)
{
    change_type(dest, source->value_type);
    Type::Copy copyFunc = source->value_type->copy;

    if (copyFunc == copy) {
        std::string msg = "Circular usage of copy() in type " + source->value_type->name;
        internal_error(msg.c_str());
    }

    if (copyFunc != NULL) {
        copyFunc(source, dest);
        return;
    }

    // Default behavior, shallow assign.
    dest->value_data = source->value_data;
}

void swap(TaggedValue* left, TaggedValue* right)
{
    Type* temp_type = left->value_type;
    TaggedValue::Data temp_data = left->value_data;
    left->value_type = right->value_type;
    left->value_data = right->value_data;
    right->value_type = temp_type;
    right->value_data = temp_data;
}

void reset(TaggedValue* value)
{
    Type* type = value->value_type;

    // Check if there is a default value defined
    TaggedValue* defaultValue = type_t::get_default_value(type);
    if (defaultValue != NULL
            && defaultValue->value_type != VOID_T) {
        copy(defaultValue, value);
        return;
    }

    // Check if the reset() function is defined
    if (type->reset != NULL) {
        type->reset(value);
        return;
    }

    // No default value, just change type to null and back
    change_type(value, NULL_T);
    change_type(value, type);
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

TaggedValue* get_index(TaggedValue* value, int index)
{
    Type::GetIndex getIndex = value->value_type->getIndex;

    // Default behavior: return NULL
    if (getIndex == NULL)
        return NULL;

    return getIndex(value, index);
}

void set_index(TaggedValue* value, int index, TaggedValue* element)
{
    Type::SetIndex setIndex = value->value_type->setIndex;

    if (setIndex == NULL) {
        std::string msg = "No setIndex function available on type " + value->value_type->name;
        internal_error(msg.c_str());
    }

    setIndex(value, index, element);
}

TaggedValue* get_field(TaggedValue* value, const char* field)
{
    Type::GetField getField = value->value_type->getField;

    if (getField == NULL)
        internal_error("No getField function available");

    return getField(value, field);
}

void set_field(TaggedValue* value, const char* field, TaggedValue* element)
{
    Type::SetField setField = value->value_type->setField;

    if (setField == NULL)
        internal_error("No setField function available");

    setField(value, field, element);
}

int num_elements(TaggedValue* value)
{
    Type::NumElements numElements = value->value_type->numElements;

    // Default behavior: return 0
    if (numElements == NULL)
        return 0;

    return numElements(value);
}

void touch(TaggedValue* value)
{
    Type::Mutate touch = value->value_type->touch;
    if (touch != NULL)
        touch(value);

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

        // Any types that are actually used become permanent
        if (!type->permanent)
            type->permanent = true;
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

TaggedValue* make_list(TaggedValue* value)
{
    change_type(value, NULL_T); // substitute for 'reset'
    change_type(value, LIST_T);
    return value;
}

void make_branch(TaggedValue* value)
{
    change_type(value, NULL_T); // substitute for 'reset'
    change_type(value, type_contents(BRANCH_TYPE));
}

void make_null(TaggedValue* value)
{
    change_type(value, NULL_T);
}

void set_int(TaggedValue* value, int i)
{
    if (!is_int(value))
        internal_error("Value is not an int");
    value->value_data.asint = i;
}

void set_float(TaggedValue* value, float f)
{
    if (!is_float(value))
        internal_error("Value is not a float");
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
        internal_error(strm.str().c_str());
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
        throw std::runtime_error("In to_float, type is not an int or float");
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

}
