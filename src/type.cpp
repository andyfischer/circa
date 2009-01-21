// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "cpp_interface.h"
#include "function.h"
#include "importing.h"
#include "list.h"
#include "runtime.h"
#include "term.h"
#include "type.h"
#include "values.h"

namespace circa {

void
Type::addMemberFunction(std::string const &name, Term *function)
{
    this->memberFunctions.bind(function, name);
}

void assert_type(Term *term, Term *type)
{
    assert(term != NULL);
    // assert(type != NULL); type may be NULL during bootstrapping

    if (term->type != type)
        throw std::runtime_error("type mismatch");
}

bool is_type(Term* term)
{
    assert(term != NULL);
    return term->type == TYPE_TYPE;
}

Type& as_type(Term *term)
{
    assert_type(term, TYPE_TYPE);
    assert(term->value != NULL);
    return *((Type*) term->value);
}

Term* quick_create_type(Branch& branch, std::string name)
{
    Term* term = create_value(&branch, TYPE_TYPE);

    if (name != "") {
        as_type(term).name = name;
        branch.bindName(term, name);
    }

    return term;
}

void unsafe_change_type(Term *term, Term *type)
{
    assert(type != NULL);

    if (term->value == NULL) {
        change_type(term, type);
        return;
    }

    term->type = type;
}

void change_type(Term *term, Term *typeTerm)
{
    assert_type(typeTerm, TYPE_TYPE);

    if (term->type == typeTerm)
        return;

    // if term->value is not NULL, it's a possible memory leak
    assert(term->value == NULL);

    term->type = typeTerm;

    Type& type = as_type(typeTerm);

    if (type.alloc == NULL) {
        throw std::runtime_error(std::string("type ") + type.name + " has no alloc function");
    }

    alloc_value(term);
}

void specialize_type(Term *term, Term *type)
{
    if (term->type == type) {
        return;
    }

    assert_type(term, ANY_TYPE);

    change_type(term, type);
}

namespace type_private {

void* empty_allocate(Term*)
{
    return NULL;
}
void empty_dealloc(void*) {}
void empty_duplicate_function(Term*,Term*) {}

}

Term* create_empty_type(Branch& branch, std::string name)
{
    Term* term = create_value(&branch, TYPE_TYPE);
    Type& type = as_type(term);
    type.alloc = type_private::empty_allocate;
    type.dealloc = type_private::empty_dealloc;
    type.duplicate = type_private::empty_duplicate_function;
    type.name = name;
    branch.bindName(term, name);
    return term;
}

void* alloc_from_type(Term* typeTerm)
{
    return as_type(typeTerm).alloc(typeTerm);
}

std::string Type::to_string(Term *caller)
{
    return std::string("<Type " + as_type(caller).name + ">");
}

void Type::typeRemapPointers(Term *term, ReferenceMap const& map)
{
    Type &type = as_type(term);

    for (unsigned int field_i=0; field_i < type.fields.size(); field_i++) {
        Field &field = type.fields[field_i];
        field.type = map.getRemapped(field.type);
    }
}

void Type::typeVisitPointers(Term *term, PointerVisitor &visitor)
{
    Type &type = as_type(term);

    for (unsigned int field_i=0; field_i < type.fields.size(); field_i++) {
        visitor.visitPointer(type.fields[field_i].type);
    }
}

void initialize_type_type(Term* typeType)
{
    typeType->value = new Type();
    as_type(typeType).name = "Type";
    as_type(typeType).alloc = cpp_interface::templated_alloc<Type>;
    as_type(typeType).dealloc = cpp_interface::templated_dealloc<Type>;
    as_type(typeType).duplicate = cpp_interface::templated_duplicate<Type>;
    as_type(typeType).toString = Type::to_string;
}

std::string to_string(Term* term)
{
    Type::ToStringFunc func = as_type(term->type).toString;

    if (func == NULL) {
        throw std::runtime_error("No toString function defined");
    } else {
        return func(term);
    }
}

std::string to_source_string(Term* term)
{
    Type::ToSourceStringFunc func = as_type(term->type).toSourceString;

    if (func == NULL) {
        throw std::runtime_error("No toSourceString function defined");
    } else {
        return func(term);
    }
}

PointerIterator* start_pointer_iterator(Term* term)
{
    Type& type = as_type(term->type);

    if (type.startPointerIterator == NULL)
        return NULL;

    return type.startPointerIterator(term);
}

} // namespace circa
