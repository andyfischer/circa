// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "kernel.h"
#include "debug.h"
#include "names.h"
#include "tagged_value.h"
#include "type.h"

#include "types/ref.h"

namespace circa {

TValue::TValue()
{
    initialize_null(this);
}

TValue::TValue(Type* type)
{
    initialize_null(this);
    create(type, this);
}

TValue::~TValue()
{
    // Deallocate this value
    set_null(this);
}

TValue::TValue(TValue const& original)
{
    initialize_null(this);
    copy(&const_cast<TValue&>(original), this);
}

TValue&
TValue::operator=(TValue const& rhs)
{
    copy(&const_cast<TValue&>(rhs), this);
    return *this;
}

void TValue::reset()
{
    circa::reset(this);
}

std::string
TValue::toString()
{
    return to_string(this);
}

TValue*
TValue::getIndex(int index)
{
    return get_index(this, index);
}

TValue*
TValue::getField(const char* fieldName)
{
    return get_field(this, fieldName);
}

TValue*
TValue::getField(std::string const& fieldName)
{
    return get_field(this, fieldName.c_str());
}

int
TValue::numElements()
{
    return num_elements(this);
}

bool
TValue::equals(TValue* rhs)
{
    return circa::equals(this, rhs);
}

int TValue::asInt()
{
    return as_int(this);
}

float TValue::asFloat()
{
    return as_float(this);
}

float TValue::toFloat()
{
    return to_float(this);
}

const char* TValue::asCString()
{
    return as_string(this).c_str();
}

std::string const& TValue::asString()
{
    return as_string(this);
}

bool TValue::asBool()
{
    return as_bool(this);
}

Term* TValue::asRef()
{
    return as_ref(this);
}

TValue TValue::fromInt(int i)
{
    TValue tv;
    set_int(&tv, i);
    return tv;
}

TValue TValue::fromFloat(float f)
{
    TValue tv;
    set_float(&tv, f);
    return tv;
}

TValue TValue::fromString(const char* s)
{
    TValue tv;
    set_string(&tv, s);
    return tv;
}

TValue TValue::fromBool(bool b)
{
    TValue tv;
    set_bool(&tv, b);
    return tv;
}

void initialize_null(TValue* value)
{
    value->value_type = &NULL_T;
    value->value_data.ptr = NULL;
}

void create(Type* type, TValue* value)
{
    set_null(value);

    value->value_type = type;

    if (type->initialize != NULL)
        type->initialize(type, value);
}

void change_type(TValue* v, Type* t)
{
    set_null(v);
    v->value_type = t;
}

void set_null(TValue* value)
{
    if (value->value_type == NULL)
        return;

    if (value->value_type->release != NULL)
        value->value_type->release(value);

    value->value_type = &NULL_T;
    value->value_data.ptr = NULL;
}

void release(TValue* value)
{
    if (value->value_type != NULL) {
        ReleaseFunc release = value->value_type->release;
        if (release != NULL)
            release(value);
    }
    value->value_type = &NULL_T;
    value->value_data.ptr = 0;
}

void cast(CastResult* result, TValue* source, Type* type, TValue* dest, bool checkOnly)
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
    result->success = true;
}

bool cast(TValue* source, Type* type, TValue* dest)
{
    CastResult result;
    cast(&result, source, type, dest, false);
    return result.success;
}

bool cast_possible(TValue* source, Type* type)
{
    CastResult result;
    cast(&result, source, type, NULL, true);
    return result.success;
}

void copy(TValue* source, TValue* dest)
{
    ca_assert(source);
    ca_assert(dest);

    if (source == dest)
        return;

    if (source->value_type->nocopy)
        internal_error("copy() called on a nocopy type");

    Type::Copy copyFunc = source->value_type->copy;

    if (copyFunc != NULL) {
        copyFunc(source->value_type, source, dest);
        ca_assert(dest->value_type == source->value_type);
        return;
    }

    // Default behavior, shallow assign.
    set_null(dest);
    dest->value_type = source->value_type;
    dest->value_data = source->value_data;
}

void swap(TValue* left, TValue* right)
{
    Type* temp_type = left->value_type;
    TValueData temp_data = left->value_data;
    left->value_type = right->value_type;
    left->value_data = right->value_data;
    right->value_type = temp_type;
    right->value_data = temp_data;
}

void move(TValue* source, TValue* dest)
{
    set_null(dest);
    dest->value_type = source->value_type;
    dest->value_data = source->value_data;
    initialize_null(source);
}

void reset(TValue* value)
{
    // Check for NULL. Most TValue functions don't do this, but reset() is
    // a convenient special case.
    if (value->value_type == NULL)
        return set_null(value);

    Type* type = value->value_type;

    // Check if the reset() function is defined
    if (type->reset != NULL) {
        type->reset(type, value);
        return;
    }

    // Default behavior: assign this value to null and create a new one.
    set_null(value);
    create(type, value);
}

void touch(TValue* value)
{
    Type::Touch touch = value->value_type->touch;
    if (touch != NULL)
        touch(value);

    // Default behavior: no-op.
}

std::string to_string(TValue* value)
{
    if (value->value_type == NULL)
        return "<type is NULL>";

    Type::ToString toString = value->value_type->toString;
    if (toString != NULL)
        return toString(value);

    std::stringstream out;
    out << "<" << name_to_string(value->value_type->name)
        << " " << value->value_data.ptr << ">";
    return out.str();
}

std::string to_string_annotated(TValue* value)
{
    if (value->value_type == NULL)
        return "<type is NULL>";

    std::stringstream out;

    out << name_to_string(value->value_type->name) << "#";

    if (is_list(value)) {
        out << "[";
        for (int i=0; i < value->numElements(); i++) {
            if (i > 0) out << ", ";
            out << to_string_annotated(value->getIndex(i));
        }
        out << "]";
    } else {
        out << to_string(value);
    }

    return out.str();
}

TValue* get_index(TValue* value, int index)
{
    Type::GetIndex getIndex = value->value_type->getIndex;

    // Default behavior: return NULL
    if (getIndex == NULL)
        return NULL;

    return getIndex(value, index);
}

void set_index(TValue* value, int index, TValue* element)
{
    Type::SetIndex setIndex = value->value_type->setIndex;

    if (setIndex == NULL) {
        std::string msg = std::string("No setIndex function available on type ")
            + name_to_string(value->value_type->name);
        internal_error(msg.c_str());
    }

    setIndex(value, index, element);
}

TValue* get_field(TValue* value, const char* field)
{
    Type::GetField getField = value->value_type->getField;

    if (getField == NULL) {
        std::string msg = std::string("No getField function available on type ")
            + name_to_string(value->value_type->name);
        internal_error(msg.c_str());
    }

    return getField(value, field);
}

void set_field(TValue* value, const char* field, TValue* element)
{
    Type::SetField setField = value->value_type->setField;

    if (setField == NULL) {
        std::string msg = std::string("No setField function available on type ")
            + name_to_string(value->value_type->name);
        internal_error(msg.c_str());
    }

    setField(value, field, element);
}

int num_elements(TValue* value)
{
    Type::NumElements numElements = value->value_type->numElements;

    // Default behavior: return 0
    if (numElements == NULL)
        return 0;

    return numElements(value);
}

bool equals(TValue* lhs, TValue* rhs)
{
    ca_assert(lhs->value_type != NULL);

    Type::Equals equals = lhs->value_type->equals;

    if (equals != NULL)
        return equals(lhs->value_type, lhs, rhs);

    // Default behavior for different types: return false
    if (lhs->value_type != rhs->value_type)
        return false;

    // Default behavor for same types: shallow comparison
    return lhs->value_data.asint == rhs->value_data.asint;
}

bool equals_string(TValue* value, const char* s)
{
    if (!is_string(value))
        return false;
    return strcmp(as_cstring(value), s) == 0;
}

bool equals_int(TValue* value, int i)
{
    if (!is_int(value))
        return false;
    return as_int(value) == i;
}

bool equals_name(TValue* value, Name name)
{
    if (!is_name(value))
        return false;
    return as_name(value) == name;
}

void set_bool(TValue* value, bool b)
{
    change_type(value, &BOOL_T);
    value->value_data.asbool = b;
}

Dict* set_dict(TValue* value)
{
    create(&DICT_T, value);
    return (Dict*) value;
}

void set_int(TValue* value, int i)
{
    change_type(value, &INT_T);
    value->value_data.asint = i;
}

void set_float(TValue* value, float f)
{
    change_type(value, &FLOAT_T);
    value->value_data.asfloat = f;
}

void set_string(TValue* value, const char* s)
{
    create(&STRING_T, value);
    *((std::string*) value->value_data.ptr) = s;
}


void set_string(TValue* value, std::string const& s)
{
    set_string(value, s.c_str());
}

List* set_list(TValue* value)
{
    set_null(value);
    create(&LIST_T, value);
    return List::checkCast(value);
}

List* set_list(TValue* value, int size)
{
    List* list = set_list(value);
    list->resize(size);
    return list;
}

void set_type(TValue* value, Type* type)
{
    set_null(value);
    value->value_type = &TYPE_T;
    value->value_data.ptr = type;
}

void set_function_pointer(TValue* value, Term* function)
{
    change_type(value, &FUNCTION_T);
    value->value_data.ptr = function;
}


void set_opaque_pointer(TValue* value, void* addr)
{
    change_type(value, &OPAQUE_POINTER_T);
    value->value_data.ptr = addr;
}
void set_branch(TValue* value, Branch* branch)
{
    change_type(value, &BRANCH_T);
    value->value_data.ptr = branch;
}

void set_pointer(TValue* value, Type* type, void* p)
{
    set_null(value);
    value->value_type = type;
    value->value_data.ptr = p;
}

void set_pointer(TValue* value, void* ptr)
{
    value->value_data.ptr = ptr;
}

int as_int(TValue* value)
{
    ca_assert(is_int(value));
    return value->value_data.asint;
}

float as_float(TValue* value)
{
    ca_assert(is_float(value));
    return value->value_data.asfloat;
}
Function* as_function(TValue* value)
{
    ca_assert(is_function(value));
    return (Function*) value->value_data.ptr;
}

bool as_bool(TValue* value)
{
    ca_assert(is_bool(value));
    return value->value_data.asbool;
}

Branch* as_branch(TValue* value)
{
    ca_assert(is_branch(value));
    return (Branch*) value->value_data.ptr;
}

void* as_opaque_pointer(TValue* value)
{
    ca_assert(value->value_type->storageType == STORAGE_TYPE_OPAQUE_POINTER);
    return value->value_data.ptr;
}

Type* as_type(TValue* value)
{
    ca_assert(is_type(value));
    return (Type*) value->value_data.ptr;
}
Term* as_function_pointer(TValue* value)
{
    ca_assert(is_function_pointer(value));
    return (Term*) value->value_data.ptr;
}
List* as_list(TValue* value)
{
    return List::checkCast(value);
}

void* get_pointer(TValue* value)
{
    return value->value_data.ptr;
}

const char* get_name_for_type(Type* type)
{
    if (type == NULL)
        return "<NULL>";
    else return name_to_string(type->name);
}

void* get_pointer(TValue* value, Type* expectedType)
{
    if (value->value_type != expectedType) {
        std::stringstream strm;
        strm << "Type mismatch in get_pointer, expected " << get_name_for_type(expectedType);
        strm << ", but value has type " << get_name_for_type(value->value_type);
        internal_error(strm.str().c_str());
    }

    return value->value_data.ptr;
}

bool is_bool(TValue* value) { return value->value_type->storageType == STORAGE_TYPE_BOOL; }
bool is_branch(TValue* value) { return value->value_type == &BRANCH_T; }
bool is_error(TValue* value) { return value->value_type == &ERROR_T; }
bool is_float(TValue* value) { return value->value_type->storageType == STORAGE_TYPE_FLOAT; }
bool is_function(TValue* value) { return value->value_type == &FUNCTION_T; }
bool is_function_pointer(TValue* value) { return value->value_type == &FUNCTION_T; }
bool is_int(TValue* value) { return value->value_type->storageType == STORAGE_TYPE_INT; }
bool is_list(TValue* value) { return value->value_type->storageType == STORAGE_TYPE_LIST; }
bool is_null(TValue* value) { return value->value_type == &NULL_T; }
bool is_opaque_pointer(TValue* value) { return value->value_type->storageType == STORAGE_TYPE_OPAQUE_POINTER; }
bool is_ref(TValue* value) { return value->value_type->storageType == STORAGE_TYPE_REF; }
bool is_string(TValue* value) { return value->value_type->storageType == STORAGE_TYPE_STRING; }
bool is_name(TValue* value) { return value->value_type == &NAME_T; }
bool is_type(TValue* value) { return value->value_type->storageType == STORAGE_TYPE_TYPE; }

bool is_number(TValue* value)
{
    return is_int(value) || is_float(value);
}

float to_float(TValue* value)
{
    if (is_int(value))
        return (float) as_int(value);
    else if (is_float(value))
        return as_float(value);
    else
        throw std::runtime_error("In to_float, type is not an int or float");
}

int to_int(TValue* value)
{
    if (is_int(value))
        return as_int(value);
    else if (is_float(value))
        return (int) as_float(value);
    else
        throw std::runtime_error("In to_float, type is not an int or float");
}

void set_transient_value(TValue* value, void* data, Type* type)
{
    set_null(value);
    value->value_data.ptr = data;
    value->value_type = type;
}
void cleanup_transient_value(TValue* value)
{
    initialize_null(value);
}


} // namespace circa
