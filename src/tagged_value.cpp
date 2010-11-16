// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "debug_valid_objects.h"
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
    init();

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
const char* TaggedValue::asCString()
{
    return as_string(this).c_str();
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

TaggedValue TaggedValue::fromInt(int i)
{
    TaggedValue tv;
    set_int(&tv, i);
    return tv;
}

TaggedValue TaggedValue::fromFloat(float f)
{
    TaggedValue tv;
    set_float(&tv, f);
    return tv;
}
TaggedValue TaggedValue::fromString(const char* s)
{
    TaggedValue tv;
    set_string(&tv, s);
    return tv;
}

TaggedValue TaggedValue::fromBool(bool b)
{
    TaggedValue tv;
    set_bool(&tv, b);
    return tv;
}

void cast(CastResult* result, TaggedValue* source, Type* type, TaggedValue* dest, bool checkOnly)
{
    if (type->cast != NULL) {
        type->cast(result, source, type, dest, checkOnly);
        return;
    }

    // Default case when the type has no handler: only allow the cast if source has the exact
    // same type

    if (source->value_type != type) {
        result->success = false;
        return;
    }

    if (checkOnly)
        return;

    copy(source, dest);
}

bool cast(TaggedValue* source, Type* type, TaggedValue* dest)
{
    CastResult result;
    cast(&result, source, type, dest, false);
    return result.success;
}

bool cast_possible(TaggedValue* source, Type* type)
{
    CastResult result;
    cast(&result, source, type, NULL, true);
    return result.success;
}

void copy(TaggedValue* source, TaggedValue* dest)
{
    ca_assert(source);
    ca_assert(dest);

    if (source == dest)
        return;

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
    // Check for NULL. Most TaggedValue functions don't do this, but reset() is
    // a convenient special case.
    if (value->value_type == NULL)
        return set_null(value);

    Type* type = value->value_type;

    // Check if there is a default value defined
    TaggedValue* defaultValue = type_t::get_default_value(type);
    if (defaultValue != NULL && !is_null(defaultValue) && defaultValue->value_type) {
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
    if (value->value_type == NULL)
        return "<type is NULL>";

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
    // type may be null
    ca_assert(v != NULL);
    debug_assert_valid_object(type, TYPE_OBJECT);
    debug_assert_valid_object(v->value_type, TYPE_OBJECT);

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
    ca_assert(lhs->value_type != NULL);

    Type::Equals equals = lhs->value_type->equals;

    if (equals != NULL)
        return equals(lhs, rhs);

    // Default behavior for different types: return false
    if (lhs->value_type != rhs->value_type)
        return false;

    // Default behavor for same types: shallow comparison
    return lhs->value_data.asint == rhs->value_data.asint;
}

TaggedValue* set_int(TaggedValue* value)
{
    change_type(value, INT_T);
    return value;
}

void set_int(TaggedValue* value, int i)
{
    change_type(value, INT_T);
    value->value_data.asint = i;
}

void set_float(TaggedValue* value, float f)
{
    change_type(value, FLOAT_T);
    value->value_data.asfloat = f;
}

void set_string(TaggedValue* value, const char* s)
{
    change_type(value, STRING_T);
    *((std::string*) value->value_data.ptr) = s;
}

void set_string(TaggedValue* value, std::string const& s)
{
    set_string(value, s.c_str());
}

void set_bool(TaggedValue* value, bool b)
{
    change_type(value, BOOL_T);
    value->value_data.asbool = b;
}

void set_ref(TaggedValue* value, Term* t)
{
    change_type(value, &as_type(REF_TYPE));
    *((Ref*) value->value_data.ptr) = t;
}

List* set_list(TaggedValue* value)
{
    change_type(value, NULL_T); // substitute for 'reset'
    change_type(value, LIST_T);
    return List::checkCast(value);
}

List* set_list(TaggedValue* value, int size)
{
    List* list = set_list(value);
    list->resize(size);
    return list;
}

void make_branch(TaggedValue* value)
{
    change_type(value, NULL_T); // substitute for 'reset'
    change_type(value, type_contents(BRANCH_TYPE));
}

void set_type(TaggedValue* value, Type* type)
{
    reset(value);
    change_type(value, TYPE_T);
    type->refCount++;
    value->value_data.ptr = type;
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

void set_pointer(TaggedValue* value, void* ptr)
{
    value->value_data.ptr = ptr;
}

int as_int(TaggedValue* value)
{
    ca_assert(is_int(value));
    return value->value_data.asint;
}

float as_float(TaggedValue* value)
{
    ca_assert(is_float(value));
    return value->value_data.asfloat;
}

std::string const& as_string(TaggedValue* value)
{
    ca_assert(is_string(value));
    return *((std::string*) value->value_data.ptr);
}

bool as_bool(TaggedValue* value)
{
    ca_assert(is_bool(value));
    return value->value_data.asbool;
}

Ref& as_ref(TaggedValue* value)
{
    ca_assert(is_ref(value));
    return *((Ref*) value->value_data.ptr);
}

Type& as_type(TaggedValue* value)
{
    ca_assert(is_type(value));
    return *((Type*) value->value_data.ptr);
}

void* get_pointer(TaggedValue* value)
{
    return value->value_data.ptr;
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

float to_float(TaggedValue* value)
{
    if (is_int(value))
        return (float) as_int(value);
    else if (is_float(value))
        return as_float(value);
    else
        throw std::runtime_error("In to_float, type is not an int or float");
}

int to_int(TaggedValue* value)
{
    if (is_int(value))
        return as_int(value);
    else if (is_float(value))
        return (int) as_float(value);
    else
        throw std::runtime_error("In to_float, type is not an int or float");
}

}
