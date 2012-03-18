// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "kernel.h"
#include "debug.h"
#include "names.h"
#include "tagged_value.h"
#include "type.h"

#include "types/ref.h"

using namespace circa;

caValue::caValue()
{
    initialize_null(this);
}

caValue::caValue(Type* type)
{
    initialize_null(this);
    create(type, this);
}

caValue::~caValue()
{
    // Deallocate this value
    set_null(this);
}

caValue::caValue(caValue const& original)
{
    initialize_null(this);
    copy(&const_cast<caValue&>(original), this);
}

caValue&
caValue::operator=(caValue const& rhs)
{
    copy(&const_cast<caValue&>(rhs), this);
    return *this;
}

void caValue::reset()
{
    circa::reset(this);
}

std::string
caValue::toString()
{
    return to_string(this);
}

caValue*
caValue::getIndex(int index)
{
    return get_index(this, index);
}

caValue*
caValue::getField(const char* fieldName)
{
    return get_field(this, fieldName);
}

caValue*
caValue::getField(std::string const& fieldName)
{
    return get_field(this, fieldName.c_str());
}

int
caValue::numElements()
{
    return num_elements(this);
}

bool
caValue::equals(caValue* rhs)
{
    return circa::equals(this, rhs);
}

int caValue::asInt()
{
    return as_int(this);
}

float caValue::asFloat()
{
    return as_float(this);
}

float caValue::toFloat()
{
    return to_float(this);
}

const char* caValue::asCString()
{
    return as_string(this).c_str();
}

std::string const& caValue::asString()
{
    return as_string(this);
}

bool caValue::asBool()
{
    return as_bool(this);
}

Term* caValue::asRef()
{
    return as_ref(this);
}

caValue caValue::fromInt(int i)
{
    caValue tv;
    set_int(&tv, i);
    return tv;
}

caValue caValue::fromFloat(float f)
{
    caValue tv;
    set_float(&tv, f);
    return tv;
}

caValue caValue::fromString(const char* s)
{
    caValue tv;
    set_string(&tv, s);
    return tv;
}

caValue caValue::fromBool(bool b)
{
    caValue tv;
    set_bool(&tv, b);
    return tv;
}

namespace circa {

void initialize_null(caValue* value)
{
    value->value_type = &NULL_T;
    value->value_data.ptr = NULL;
}

void create(Type* type, caValue* value)
{
    set_null(value);

    value->value_type = type;

    if (type->initialize != NULL)
        type->initialize(type, value);
}

void change_type(caValue* v, Type* t)
{
    set_null(v);
    v->value_type = t;
}

void set_null(caValue* value)
{
    if (value->value_type == NULL)
        return;

    if (value->value_type->release != NULL)
        value->value_type->release(value);

    value->value_type = &NULL_T;
    value->value_data.ptr = NULL;
}

void release(caValue* value)
{
    if (value->value_type != NULL) {
        ReleaseFunc release = value->value_type->release;
        if (release != NULL)
            release(value);
    }
    value->value_type = &NULL_T;
    value->value_data.ptr = 0;
}

void cast(CastResult* result, caValue* source, Type* type, caValue* dest, bool checkOnly)
{
    if (type->cast != NULL) {
        type->cast(result, source, type, dest, checkOnly);
        return;
    }

    // Default case when the type has no cast handler: only allow the cast if source has
    // the exact same type.

    if (source->value_type != type) {
        result->success = false;
        return;
    }

    if (checkOnly)
        return;

    copy(source, dest);
    result->success = true;
}

bool cast(caValue* source, Type* type, caValue* dest)
{
    CastResult result;
    cast(&result, source, type, dest, false);
    return result.success;
}

bool cast_possible(caValue* source, Type* type)
{
    CastResult result;
    cast(&result, source, type, NULL, true);
    return result.success;
}

void copy(caValue* source, caValue* dest)
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

void swap(caValue* left, caValue* right)
{
    Type* temp_type = left->value_type;
    caValueData temp_data = left->value_data;
    left->value_type = right->value_type;
    left->value_data = right->value_data;
    right->value_type = temp_type;
    right->value_data = temp_data;
}

void move(caValue* source, caValue* dest)
{
    set_null(dest);
    dest->value_type = source->value_type;
    dest->value_data = source->value_data;
    initialize_null(source);
}

void reset(caValue* value)
{
    // Check for NULL. Most caValue functions don't do this, but reset() is
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

void touch(caValue* value)
{
    Type::Touch touch = value->value_type->touch;
    if (touch != NULL)
        touch(value);

    // Default behavior: no-op.
}

std::string to_string(caValue* value)
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

std::string to_string_annotated(caValue* value)
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

caValue* get_index(caValue* value, int index)
{
    Type::GetIndex getIndex = value->value_type->getIndex;

    // Default behavior: return NULL
    if (getIndex == NULL)
        return NULL;

    return getIndex(value, index);
}

void set_index(caValue* value, int index, caValue* element)
{
    Type::SetIndex setIndex = value->value_type->setIndex;

    if (setIndex == NULL) {
        std::string msg = std::string("No setIndex function available on type ")
            + name_to_string(value->value_type->name);
        internal_error(msg.c_str());
    }

    setIndex(value, index, element);
}

caValue* get_field(caValue* value, const char* field)
{
    Type::GetField getField = value->value_type->getField;

    if (getField == NULL) {
        std::string msg = std::string("No getField function available on type ")
            + name_to_string(value->value_type->name);
        internal_error(msg.c_str());
    }

    return getField(value, field);
}

void set_field(caValue* value, const char* field, caValue* element)
{
    Type::SetField setField = value->value_type->setField;

    if (setField == NULL) {
        std::string msg = std::string("No setField function available on type ")
            + name_to_string(value->value_type->name);
        internal_error(msg.c_str());
    }

    setField(value, field, element);
}

int num_elements(caValue* value)
{
    Type::NumElements numElements = value->value_type->numElements;

    // Default behavior: return 0
    if (numElements == NULL)
        return 0;

    return numElements(value);
}

bool equals(caValue* lhs, caValue* rhs)
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

bool equals_string(caValue* value, const char* s)
{
    if (!is_string(value))
        return false;
    return strcmp(as_cstring(value), s) == 0;
}

bool equals_int(caValue* value, int i)
{
    if (!is_int(value))
        return false;
    return as_int(value) == i;
}

bool equals_name(caValue* value, Name name)
{
    if (!is_name(value))
        return false;
    return as_name(value) == name;
}

void set_bool(caValue* value, bool b)
{
    change_type(value, &BOOL_T);
    value->value_data.asbool = b;
}

Dict* set_dict(caValue* value)
{
    create(&DICT_T, value);
    return (Dict*) value;
}

void set_int(caValue* value, int i)
{
    change_type(value, &INT_T);
    value->value_data.asint = i;
}

void set_float(caValue* value, float f)
{
    change_type(value, &FLOAT_T);
    value->value_data.asfloat = f;
}

void set_string(caValue* value, const char* s)
{
    create(&STRING_T, value);
    *((std::string*) value->value_data.ptr) = s;
}

void set_string(caValue* value, std::string const& s)
{
    set_string(value, s.c_str());
}

List* set_list(caValue* value)
{
    set_null(value);
    create(&LIST_T, value);
    return List::checkCast(value);
}

List* set_list(caValue* value, int size)
{
    List* list = set_list(value);
    list->resize(size);
    return list;
}

void set_type(caValue* value, Type* type)
{
    set_null(value);
    value->value_type = &TYPE_T;
    value->value_data.ptr = type;
}

void set_function(caValue* value, Function* function)
{
    set_null(value);
    value->value_type = &FUNCTION_T;
    value->value_data.ptr = function;
}

void set_opaque_pointer(caValue* value, void* addr)
{
    change_type(value, &OPAQUE_POINTER_T);
    value->value_data.ptr = addr;
}
void set_branch(caValue* value, Branch* branch)
{
    change_type(value, &BRANCH_T);
    value->value_data.ptr = branch;
}

void set_pointer(caValue* value, Type* type, void* p)
{
    set_null(value);
    value->value_type = type;
    value->value_data.ptr = p;
}

void set_pointer(caValue* value, void* ptr)
{
    value->value_data.ptr = ptr;
}

int as_int(caValue* value)
{
    ca_assert(is_int(value));
    return value->value_data.asint;
}

float as_float(caValue* value)
{
    ca_assert(is_float(value));
    return value->value_data.asfloat;
}
Function* as_function(caValue* value)
{
    ca_assert(is_function(value));
    return (Function*) value->value_data.ptr;
}

bool as_bool(caValue* value)
{
    ca_assert(is_bool(value));
    return value->value_data.asbool;
}

Branch* as_branch(caValue* value)
{
    ca_assert(is_branch(value));
    return (Branch*) value->value_data.ptr;
}

void* as_opaque_pointer(caValue* value)
{
    ca_assert(value->value_type->storageType == STORAGE_TYPE_OPAQUE_POINTER);
    return value->value_data.ptr;
}

Type* as_type(caValue* value)
{
    ca_assert(is_type(value));
    return (Type*) value->value_data.ptr;
}
List* as_list(caValue* value)
{
    return List::checkCast(value);
}

void* get_pointer(caValue* value)
{
    return value->value_data.ptr;
}

const char* get_name_for_type(Type* type)
{
    if (type == NULL)
        return "<NULL>";
    else return name_to_string(type->name);
}

void* get_pointer(caValue* value, Type* expectedType)
{
    if (value->value_type != expectedType) {
        std::stringstream strm;
        strm << "Type mismatch in get_pointer, expected " << get_name_for_type(expectedType);
        strm << ", but value has type " << get_name_for_type(value->value_type);
        internal_error(strm.str().c_str());
    }

    return value->value_data.ptr;
}

bool is_bool(caValue* value) { return value->value_type->storageType == STORAGE_TYPE_BOOL; }
bool is_branch(caValue* value) { return value->value_type == &BRANCH_T; }
bool is_error(caValue* value) { return value->value_type == &ERROR_T; }
bool is_float(caValue* value) { return value->value_type->storageType == STORAGE_TYPE_FLOAT; }
bool is_function(caValue* value) { return value->value_type == &FUNCTION_T; }
bool is_function_pointer(caValue* value) { return value->value_type == &FUNCTION_T; }
bool is_int(caValue* value) { return value->value_type->storageType == STORAGE_TYPE_INT; }
bool is_list(caValue* value) { return value->value_type->storageType == STORAGE_TYPE_LIST; }
bool is_null(caValue* value) { return value->value_type == &NULL_T; }
bool is_opaque_pointer(caValue* value) { return value->value_type->storageType == STORAGE_TYPE_OPAQUE_POINTER; }
bool is_ref(caValue* value) { return value->value_type->storageType == STORAGE_TYPE_REF; }
bool is_string(caValue* value) { return value->value_type->storageType == STORAGE_TYPE_STRING; }
bool is_name(caValue* value) { return value->value_type == &NAME_T; }
bool is_type(caValue* value) { return value->value_type->storageType == STORAGE_TYPE_TYPE; }

bool is_number(caValue* value)
{
    return is_int(value) || is_float(value);
}

float to_float(caValue* value)
{
    if (is_int(value))
        return (float) as_int(value);
    else if (is_float(value))
        return as_float(value);
    else
        throw std::runtime_error("In to_float, type is not an int or float");
}

int to_int(caValue* value)
{
    if (is_int(value))
        return as_int(value);
    else if (is_float(value))
        return (int) as_float(value);
    else
        throw std::runtime_error("In to_float, type is not an int or float");
}

void set_transient_value(caValue* value, void* data, Type* type)
{
    set_null(value);
    value->value_data.ptr = data;
    value->value_type = type;
}
void cleanup_transient_value(caValue* value)
{
    initialize_null(value);
}


} // namespace circa
