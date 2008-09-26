// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "cpp_interface.h"
#include "errors.h"
#include "function.h"
#include "list.h"
#include "operations.h"
#include "term.h"
#include "type.h"
#include "values.h"

namespace circa {

void CompoundType__dealloc(Term* caller);

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
    return as_list(ct)[1];
}

Term* get_field_type(Term* field)
{
    assert(field != NULL);
    return as_list(field)[0];
}

void compound_type_append_field(Term* ct, Term *type, std::string const& name)
{
    Term* field = as_list(get_compound_type_fields(ct)).appendSlot(LIST_TYPE);
    as_list(field).appendSlot(REFERENCE_TYPE)->asRef() = type;
    as_list(field).appendSlot(STRING_TYPE)->asString() = name;
}

Term* get_field_name(Term *field)
{
    assert(field != NULL);
    return as_list(field)[1];
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
    assert(is_list(term));
    return as_list(term)[0];
}

Term* get_as(Term *term, Term *type)
{
    assert(term != NULL);

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
    assert(term != NULL);

    // Special case during bootstrapping.
    if (CURRENTLY_BOOTSTRAPPING && type == NULL)
        return true;

    term = get_as(term, type);

    return term != NULL;
}

void assert_instance(Term *term, Term *type)
{
    assert(term != NULL);

    if (!is_instance(term, type))
        throw errors::TypeError(term, type);
}

bool is_type(Term *term)
{
    return is_instance(term, TYPE_TYPE);
}

Type* as_type(Term *term)
{
    assert(term != NULL);
    assert_instance(term, TYPE_TYPE);
    term = get_as(term, TYPE_TYPE);
    return (Type*) term->value;
}

Term* get_field(Term *term, std::string const& fieldName)
{
    assert_instance(term->type, COMPOUND_TYPE);

    // Look into term's type, find the index of this field name
    List& fields = as_list(get_compound_type_fields(term->type));

    int index = 0;
    for (; index < fields.count(); index++) {
        if (as_string(get_field_name(fields[index])) == fieldName)
            break;
    }

    if (index == fields.count())
        return NULL;

    return as_list(term)[index];
}

void Type_alloc(Term *caller)
{
    caller->value = new Type();
}

void Type_dealloc(Term* caller)
{
    delete as_type(caller);
}

std::string Type_toString(Term *caller)
{
    return std::string("<Type " + as_type(caller)->name + ">");
}


void CompoundType__alloc(Term *caller)
{
    cpp_interface::templated_alloc<List>(caller);

    List& fields = as_list(get_compound_type_fields(caller->type));

    for (int i=0; i < fields.count(); i++)
        as_list(caller).appendSlot(get_field_type(fields[i])->asRef());
}

void CompoundType__dealloc(Term *caller)
{
    cpp_interface::templated_dealloc<List>(caller);
}

void CompoundType__create_compound_type__evaluate(Term* caller)
{
    std::string name = as_string(caller->inputs[0]);
    *as_type(caller) = *as_type(LIST_TYPE);
    as_type(caller)->alloc = CompoundType__alloc;
    as_type(caller)->dealloc = CompoundType__dealloc;
}

void CompoundType__append_field__evaluate(Term *caller)
{
    recycle_value(caller->inputs[0], caller);
    Term* type = caller->inputs[1];
    std::string name = as_string(caller->inputs[2]);

    compound_type_append_field(caller, type, name);
}

/*
void set_member_function(Term *type, std::string name, Term *function)
{
    Type* typeData = as_type(type);
    as_function(function);

    typeData->memberFunctions.bind(function, name);
}

Term* get_member_function(Term* type, std::string name)
{
    return as_type(type)->memberFunctions[name];
}*/

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

void initialize_compound_types(Branch* kernel)
{
    /* 
        type CompoundType {
            Ref parent
            List<Field> fields
        }
        type Field {
            Ref type
            String name
        }
    */

    COMPOUND_TYPE = create_constant(kernel, LIST_TYPE);
    kernel->bindName(COMPOUND_TYPE, "CompoundType");

    // parent instance, type
    as_list(COMPOUND_TYPE).appendSlot(TYPE_TYPE);

    // fields
    as_list(COMPOUND_TYPE).appendSlot(LIST_TYPE);
    compound_type_append_field(COMPOUND_TYPE, TYPE_TYPE, "parent");
    compound_type_append_field(COMPOUND_TYPE, LIST_TYPE, "fields");

    // bootstrap
    COMPOUND_TYPE->type = COMPOUND_TYPE;

    as_type(COMPOUND_TYPE)->name = "CompoundType";
    *as_type(COMPOUND_TYPE) = *as_type(LIST_TYPE);
    as_type(COMPOUND_TYPE)->alloc = CompoundType__alloc;
    as_type(COMPOUND_TYPE)->dealloc = CompoundType__dealloc;


    quick_create_function(kernel, "create-compound-type",
            CompoundType__create_compound_type__evaluate,
            ReferenceList(STRING_TYPE),
            COMPOUND_TYPE);
    quick_create_function(kernel, "compound-type-append-field",
            CompoundType__append_field__evaluate,
            ReferenceList(COMPOUND_TYPE, TYPE_TYPE, STRING_TYPE),
            COMPOUND_TYPE);
}

} // namespace circa
