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

int& as_int(Term* term)
{
    assert_type(term, INT_TYPE);
    assert(term->value != NULL);

    return *((int*) term->value);
}

float& as_float(Term* term)
{
    assert_type(term, FLOAT_TYPE);
    assert(term->value != NULL);

    return *((float*) term->value);
}

bool& as_bool(Term* term)
{
    assert_type(term, BOOL_TYPE);
    assert(term->value != NULL);

    return *((bool*) term->value);
}

std::string& as_string(Term* term)
{
    assert_type(term, STRING_TYPE);
    assert(term->value != NULL);

    if (term->value == NULL)
        throw std::runtime_error("NULL pointer in as_string");

    return *((std::string*) term->value);
}

namespace ref_type {
    void alloc(Term* term) {
        term->value = NULL;
    }

    void dealloc(Term* term) {
        term->value = NULL;
    }

    void visitPointers(Term* term, PointerVisitor& visitor)
    {
        visitor.visitPointer((Term*) term->value);
    }

    void remapPointers(Term* term, ReferenceMap const& map)
    {
        term->value = map.getRemapped((Term*) term->value);
    }
}

Term*& as_ref(Term* term)
{
    return (Term*&) term->value;
}

void
Type::addMemberFunction(std::string const &name, Term *function)
{
    this->memberFunctions.bind(function, name);
}

struct CompoundValue
{
    int signature;
    Branch branch;
    ReferenceList fields;

    // Member functions
    CompoundValue() : signature(COMPOUND_TYPE_SIGNATURE) {}

    Term* appendSlot(Term* type) {
        Term* newTerm = create_var(&branch, type);
        fields.append(newTerm);
        newTerm->stealingOk = false;
        return newTerm;
    }

    // Static functions
    static void alloc(Term* term)
    {
        CompoundValue *value = new CompoundValue();
        term->value = value;

        // create a slot for each field
        Type& type = as_type(term->type);
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
        Type& output = as_type(term);

        output.name = name;
        output.alloc = alloc;
        output.dealloc = dealloc;
    }

    static void append_field(Term* term)
    {
        recycle_value(term->inputs[0], term);
        Type& output = as_type(term);
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
        Type& type = as_type(term->inputs[0]->type);

        int index = type.findField(fieldName);

        if (index == -1) {
            error_occured(term, std::string("field \'")+fieldName+"\' not found");
            return;
        }

        assert(index >= 0);

        Term* field = value.fields[index];
        specialize_type(term, field->type);

        if (field->stealingOk)
            std::cout << "warning: stealing from a field" << std::endl;

        recycle_value(field, term);
    }
};

Term* get_field(Term *term, std::string const& fieldName)
{
    assert(CompoundValue::is_compound_value(term));
    CompoundValue *value = (CompoundValue*) term->value;
    Type& type = as_type(term->type);
    int index = type.findField(fieldName);
    if (index == -1)
        return NULL;
    return value->fields[index];
}

void assert_type(Term *term, Term *type)
{
    assert(term != NULL);
    // assert(type != NULL); type may be NULL during bootstrapping

    if (term->type != type)
        throw std::runtime_error("type mismatch");
        //throw errors::TypeError(term, type);
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

Term* quick_create_type(Branch* branch, std::string name)
{
    Term* term = create_var(branch, TYPE_TYPE);

    if (name != "") {
        as_type(term).name = name;
        branch->bindName(term, name);
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

    type.alloc(term);

    if (type.init != NULL) 
        type.init(term);
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

void empty_function(Term*) {}
void empty_duplicate_function(Term*,Term*) {}

}

Term* create_empty_type(Branch& branch, std::string name)
{
    Term* term = create_var(&branch, TYPE_TYPE);
    Type& type = as_type(term);
    type.alloc = type_private::empty_function;
    type.dealloc = type_private::empty_function;
    type.duplicate = type_private::empty_duplicate_function;
    type.name = name;
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
    assign_from_cpp_type<Type>(as_type(typeType));
    as_type(typeType).toString = Type::to_string;
}

void initialize_primitive_types(Branch* kernel)
{
    STRING_TYPE = quick_create_cpp_type<std::string>(*KERNEL, "string");
    as_type(STRING_TYPE).equals = cpp_interface::templated_equals<std::string>;
    as_type(STRING_TYPE).toString = primitives::string_t::to_string;

    INT_TYPE = quick_create_cpp_type<int>(*KERNEL, "int");
    as_type(INT_TYPE).equals = cpp_interface::templated_equals<int>;
    as_type(INT_TYPE).toString = primitives::int_t::to_string;

    FLOAT_TYPE = quick_create_cpp_type<float>(*KERNEL, "float");
    as_type(FLOAT_TYPE).equals = cpp_interface::templated_equals<float>;
    as_type(FLOAT_TYPE).toString = primitives::float_t::to_string;

    BOOL_TYPE = quick_create_cpp_type<bool>(*KERNEL, "bool");
    as_type(BOOL_TYPE).equals = cpp_interface::templated_equals<bool>;
    as_type(BOOL_TYPE).toString = primitives::bool_t::to_string;

    ANY_TYPE = create_empty_type(*KERNEL, "any");
    VOID_TYPE = create_empty_type(*KERNEL, "void");
    REFERENCE_TYPE = quick_create_type(KERNEL, "Reference");
    as_type(REFERENCE_TYPE).alloc = ref_type::alloc;
    as_type(REFERENCE_TYPE).dealloc = ref_type::dealloc;
    as_type(REFERENCE_TYPE).visitPointers = ref_type::visitPointers;
    as_type(REFERENCE_TYPE).remapPointers = ref_type::remapPointers;
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
