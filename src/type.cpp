// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "builtins.h"
#include "errors.h"
#include "function.h"
#include "operations.h"
#include "term.h"
#include "type.h"

namespace circa {

Type::Type()
  : name(""),
    parentType(NULL),
    alloc(NULL),
    dealloc(NULL),
    duplicate(NULL),
    equals(NULL),
    compare(NULL),
    remapPointers(NULL),
    toString(NULL)
{
}

void
Type::addMemberFunction(std::string const& name, Term* function)
{
    // make sure argument 0 of the function matches this type
    if (as_type(as_function(function)->inputTypes[0]) != this)
        throw errors::InternalError("argument 0 of function doesn't match this type");

    this->memberFunctions.bind(function, name);
}

bool is_instance(Term* term, Term* type)
{
    // Special case during bootstrapping.
    if (CURRENTLY_BOOTSTRAPPING && type == NULL)
        return true;

    Term* actualType = term->type;

    while (actualType != NULL) {

        if (actualType == type)
            return true;

        actualType = as_type(actualType)->parentType;
    }

    return false;
}

void assert_instance(Term* term, Term* type)
{
    if (!is_instance(term, type))
        throw errors::TypeError(term, type);
}

bool is_type(Term* term)
{
    return is_instance(term, TYPE_TYPE);
}

Type* as_type(Term* term)
{
    assert_instance(term, TYPE_TYPE);
    return (Type*) term->value;
}

void Type_alloc(Term* caller)
{
    caller->value = new Type();
}

std::string Type_toString(Term* caller)
{
    return std::string("<Type " + as_type(caller)->name + ">");
}

void set_member_function(Term* type, std::string name, Term* function)
{
    Type* typeData = as_type(type);
    as_function(function);

    typeData->memberFunctions.bind(function, name);
}

Term* get_member_function(Term* type, std::string name)
{
    return as_type(type)->memberFunctions[name];
}

} // namespace circa
