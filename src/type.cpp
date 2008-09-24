// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "errors.h"
#include "function.h"
#include "list.h"
#include "operations.h"
#include "term.h"
#include "type.h"

namespace circa {

Type::Type()
  : name(""),
    dataSize(sizeof(void*)),
    alloc(NULL),
    init(NULL),
    dealloc(NULL),
    duplicate(NULL),
    equals(NULL),
    compare(NULL),
    remapPointers(NULL),
    toString(NULL)
{
}

void
Type::addMemberFunction(std::string const &name, Term *function)
{
    // make sure argument 0 of the function matches this type
    if (as_type(as_function(function)->inputTypes[0]) != this)
        throw errors::InternalError("argument 0 of function doesn't match this type");

    this->memberFunctions.bind(function, name);
}

Term* get_compound_type_fields(Term *ct)
{
    assert(ct != NULL);
    assert(ct->type == COMPOUND_TYPE);
    return as_list(ct)[1];
}

Term* get_parent_type(Term *type)
{
    assert(type != NULL);
    assert(type->type == COMPOUND_TYPE);

    Term* parent_field = as_list(get_compound_type_fields(type))[0];

    return as_list(parent_field)[0]->asRef();
}

Term* get_parent(Term *term)
{
    assert(term != NULL);
    assert(term->type->type == COMPOUND_TYPE);
    return as_list(term)[0];
}

Term* get_as(Term *term, Term *type)
{
    if (term->type == type)
        return term;

    if (term->type->type != COMPOUND_TYPE)
        return NULL;

    Term* parent = get_parent(term);
    if (parent == NULL)
        return NULL;

    return get_as(parent, get_parent_type(term->type));
}

bool is_instance(Term *term, Term *type)
{
    // Special case during bootstrapping.
    if (CURRENTLY_BOOTSTRAPPING && type == NULL)
        return true;

    term = get_as(term, type);

    return term != NULL;
}

void assert_instance(Term *term, Term *type)
{
    if (!is_instance(term, type))
        throw errors::TypeError(term, type);
}

bool is_type(Term *term)
{
    return is_instance(term, TYPE_TYPE);
}

Type* as_type(Term *term)
{
    term = get_as(term, TYPE_TYPE);
    assert_instance(term, TYPE_TYPE);
    return (Type*) term->value;
}

void Type_alloc(Term *caller)
{
    caller->value = new Type();
}

void Type_dealloc(Term* caller)
{
    // todo
}

std::string Type_toString(Term *caller)
{
    return std::string("<Type " + as_type(caller)->name + ">");
}

void set_member_function(Term *type, std::string name, Term *function)
{
    Type* typeData = as_type(type);
    as_function(function);

    typeData->memberFunctions.bind(function, name);
}

Term* get_member_function(Term* type, std::string name)
{
    return as_type(type)->memberFunctions[name];
}

void unsafe_change_type(Term *term, Term *type)
{
    if (term->value == NULL) {
        change_type(term, type);
        return;
    }

    term->type = type;
}

void change_type(Term *term, Term *typeTerm)
{
    if (term->type == typeTerm)
        return;

    if (term->value != NULL)
        throw errors::InternalError("value is not NULL in change_type (possible memory leak)");

    Term* oldType = term->type;

    term->type = typeTerm;

    Type *type = as_type(typeTerm);

    if (type->alloc == NULL)
        throw errors::InternalError(string("type ") + type->name + " has no alloc function");

    type->alloc(term);

    if (type->init != NULL) 
        type->init(term);
}

void specialize_type(Term *term, Term *type)
{
    if (term->type == type) {
        return;
    }

    if (term->type != ANY_TYPE)
        throw errors::TypeError(term, ANY_TYPE);

    change_type(term, type);
}

namespace type_private {

void empty_function(Term*) {}

}

Term* create_empty_type(Branch* branch)
{
    Term* term = create_constant(branch, TYPE_TYPE);
    Type* type = as_type(term);
    type->alloc = type_private::empty_function;
    type->dealloc = type_private::empty_function;
    return term;
}

} // namespace circa
