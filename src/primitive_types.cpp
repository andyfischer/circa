// Copyright 2008 Andrew Fischer

#include "common_headers.h"
#include "cpp_interface.h"
#include "essentials.h"
#include "primitive_types.h"

namespace circa {

void empty_function(Term*) { }
void empty_duplicate_function(Term*,Term*) { }

int& as_int(Term* t)
{
    if (t->type != INT_TYPE)
        throw errors::TypeError(t, INT_TYPE);

    return *((int*) t->value);
}

float& as_float(Term* t)
{
    if (t->type != FLOAT_TYPE)
        throw errors::TypeError(t, FLOAT_TYPE);

    return *((float*) t->value);
}

bool& as_bool(Term* t)
{
    if (t->type != BOOL_TYPE)
        throw errors::TypeError(t, BOOL_TYPE);

    return *((bool*) t->value);
}

string& as_string(Term* t)
{
    if (t->type != STRING_TYPE)
        throw errors::TypeError(t, STRING_TYPE);

    if (t->value == NULL)
        throw errors::InternalError("NULL pointer in as_string");

    return *((string*) t->value);
}

std::string int__toString(Term* term)
{
    std::stringstream strm;
    strm << as_int(term);
    return strm.str();
}

std::string float__toString(Term* term)
{
    std::stringstream strm;
    strm << as_float(term);
    return strm.str();
}

std::string string__toString(Term* term)
{
    return as_string(term);
}

void bool_alloc(Term* caller)
{
    caller->value = new bool;
}

void bool_dealloc(Term* caller)
{
    delete (bool*) caller->value;
}

void bool_duplicate(Term* source, Term* dest)
{
    bool_alloc(dest);
    as_bool(dest) = as_bool(source);
}

std::string bool__toString(Term* term)
{
    if (as_bool(term))
        return "true";
    else
        return "false";
}

void reference_alloc(Term* caller)
{
    caller->value = NULL;
}
void reference_dealloc(Term* caller)
{
    caller->value = NULL;
}
Term*& as_ref(Term* term)
{
    return (Term*&) term->value;
}
void reference_duplicate(Term* source, Term* dest)
{
    dest->value = source->value;
}

void initialize_primitive_types(Branch* kernel)
{
    STRING_TYPE = quick_create_cpp_type<std::string>(KERNEL, "string");
    as_type(STRING_TYPE)->equals = cpp_interface::templated_equals<std::string>;
    as_type(STRING_TYPE)->toString = string__toString;

    INT_TYPE = quick_create_cpp_type<int>(KERNEL, "int");
    as_type(INT_TYPE)->equals = cpp_interface::templated_equals<int>;
    as_type(INT_TYPE)->toString = int__toString;

    FLOAT_TYPE = quick_create_cpp_type<float>(KERNEL, "float");
    as_type(FLOAT_TYPE)->equals = cpp_interface::templated_equals<float>;
    as_type(FLOAT_TYPE)->toString = float__toString;

    BOOL_TYPE = quick_create_cpp_type<bool>(KERNEL, "bool");
    as_type(BOOL_TYPE)->toString = bool__toString;

    ANY_TYPE = quick_create_type(KERNEL, "any",
            empty_function, empty_function, NULL);
    VOID_TYPE = quick_create_type(KERNEL, "void",
            empty_function, empty_function, empty_duplicate_function);
    REFERENCE_TYPE = quick_create_type(KERNEL, "Reference",
            reference_alloc,
            reference_dealloc,
            reference_duplicate);
}

} // namespace circa
