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

std::string& as_string(Term* t)
{
    if (t->type != STRING_TYPE)
        throw errors::TypeError(t, STRING_TYPE);

    if (t->value == NULL)
        throw errors::InternalError("NULL pointer in as_string");

    return *((std::string*) t->value);
}

Term*& as_ref(Term* term)
{
    return (Term*&) term->value;
}

void
Type::addMemberFunction(std::string const &name, Term *function)
{
    // make sure argument 0 of the function matches this type
    if (as_type(as_function(function).inputTypes[0]) != this)
        throw errors::InternalError("argument 0 of function doesn't match this type");

    this->memberFunctions.bind(function, name);
}


struct CompoundValue {

    int signature;
    Branch branch;
    ReferenceList fields;

    // Member functions
    CompoundValue() : signature(COMPOUND_TYPE_SIGNATURE) {}

    Term* appendSlot(Term* type) {
        Term* newTerm = create_constant(&branch, type);
        fields.append(newTerm);
        return newTerm;
    }

    // Static functions
    static void alloc(Term* term)
    {
        CompoundValue *value = new CompoundValue();
        term->value = value;

        // create a slot for each field
        Type& type = *as_type(term->type);
        int numFields = (int) type.fields.size();

        for (int f=0; f < numFields; f++)
            value->appendSlot(type.fields[f].type);
    }

    static void dealloc(Term* term)
    {
        delete (CompoundValue*) term->value;
        term->value = NULL;
    }

    static void create_compound_type(Term* term)
    {
        std::string name = as_string(term->inputs[0]);
        Type& output = *as_type(term);

        output.name = name;
        output.alloc = alloc;
        output.dealloc = dealloc;
    }

    static void append_field(Term* term)
    {
        recycle_value(term->inputs[0], term);
        Type& output = *as_type(term);
        as_type(term->inputs[1]);
        Term* fieldType = term->inputs[1];
        std::string fieldName = as_string(term->inputs[2]);
        output.addField(fieldType, fieldName);
    }

    static bool is_compound_value(Term *term)
    {
        assert(term != NULL);
        assert(term->value != NULL);
        return ((CompoundValue*) term->value)->signature == COMPOUND_TYPE_SIGNATURE;
    }

    static CompoundValue& as_compound_value(Term *term)
    {
        assert(is_compound_value(term));
        return *((CompoundValue*) term->value);
    }

    static void get_field(Term* term)
    {
        CompoundValue &value = as_compound_value(term->inputs[0]);
        std::string fieldName = as_string(term->inputs[1]);
        Type& type = *as_type(term->inputs[0]->type);

        int index = type.findField(fieldName);

        if (index == -1) {
            std::cout << "field not found" << std::endl;
            term->pushError(std::string("field \'")+fieldName+"\' not found");
            return;
        }

        assert(index >= 0);

        Term* field = value.fields[index];
        specialize_type(term, field->type);
        recycle_value(field, term);
    }
};

Term* get_field(Term *term, std::string const& fieldName)
{
    assert(CompoundValue::is_compound_value(term));
    CompoundValue *value = (CompoundValue*) term->value;
    Type& type = *as_type(term->type);
    int index = type.findField(fieldName);
    if (index == -1)
        return NULL;
    return value->fields[index];
}

bool is_instance(Term *term, Term *type)
{
    assert(term != NULL);

    return term->type == type;
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
    assert_instance(term, TYPE_TYPE);
    return ((Type*) term->value);
}

Term* quick_create_type(Branch* branch, std::string name)
{
    Term* term = create_constant(branch, TYPE_TYPE);
    as_type(term)->name = name;
    branch->bindName(term, name);
    return term;
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

    // if term->value is not NULL, it's a possible memory leak
    assert(term->value == NULL);

    term->type = typeTerm;

    Type *type = as_type(typeTerm);

    if (type->alloc == NULL) {
        throw errors::InternalError(std::string("type ") + type->name + " has no alloc function");
    }

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
void empty_duplicate_function(Term*,Term*) {}

}

Term* create_empty_type(Branch& branch, std::string name)
{
    Term* term = create_constant(&branch, TYPE_TYPE);
    Type* type = as_type(term);
    type->alloc = type_private::empty_function;
    type->dealloc = type_private::empty_function;
    type->name = name;
    branch.bindName(term, name);
    return term;
}

namespace primitives {
    namespace int_t {
        std::string to_string(Term* term)
        {
            std::stringstream strm;
            strm << as_int(term);
            return strm.str();
        }
    }

    namespace float_t {
        std::string to_string(Term* term)
        {
            std::stringstream strm;
            strm << as_float(term);
            return strm.str();
        }
    }

    namespace string_t {
        std::string to_string(Term* term)
        {
            return as_string(term);
        }
    }

    namespace bool_t {
        std::string to_string(Term* term)
        {
            if (as_bool(term))
                return "true";
            else
                return "false";
        }
    }

} // namespace primitives

std::string Type::to_string(Term *caller)
{
    return std::string("<Type " + as_type(caller)->name + ">");
}

void initialize_type_type(Term* typeType)
{
    typeType->value = new Type();
    as_type(typeType)->name = "Type";
    assign_from_cpp_type<Type>(*as_type(typeType));
    as_type(typeType)->toString = Type::to_string;
}

void initialize_primitive_types(Branch* kernel)
{
    STRING_TYPE = quick_create_cpp_type<std::string>(KERNEL, "string");
    as_type(STRING_TYPE)->equals = cpp_interface::templated_equals<std::string>;
    as_type(STRING_TYPE)->toString = primitives::string_t::to_string;

    INT_TYPE = quick_create_cpp_type<int>(KERNEL, "int");
    as_type(INT_TYPE)->equals = cpp_interface::templated_equals<int>;
    as_type(INT_TYPE)->toString = primitives::int_t::to_string;

    FLOAT_TYPE = quick_create_cpp_type<float>(KERNEL, "float");
    as_type(FLOAT_TYPE)->equals = cpp_interface::templated_equals<float>;
    as_type(FLOAT_TYPE)->toString = primitives::float_t::to_string;

    BOOL_TYPE = quick_create_cpp_type<bool>(KERNEL, "bool");
    as_type(BOOL_TYPE)->equals = cpp_interface::templated_equals<bool>;
    as_type(BOOL_TYPE)->toString = primitives::bool_t::to_string;

    ANY_TYPE = create_empty_type(*KERNEL, "any");
    VOID_TYPE = create_empty_type(*KERNEL, "void");
    REFERENCE_TYPE = quick_create_cpp_type<Term*>(KERNEL, "Reference");
}

void initialize_compound_types(Branch* kernel)
{
    quick_create_function(kernel, "create-compound-type",
            CompoundValue::create_compound_type,
            ReferenceList(STRING_TYPE),
            TYPE_TYPE);
    quick_create_function(kernel, "compound-type-append-field",
            CompoundValue::append_field,
            ReferenceList(TYPE_TYPE, TYPE_TYPE, STRING_TYPE),
            TYPE_TYPE);
    quick_create_function(kernel, "get-field",
            CompoundValue::get_field,
            ReferenceList(ANY_TYPE, STRING_TYPE),
            ANY_TYPE);
}

} // namespace circa
