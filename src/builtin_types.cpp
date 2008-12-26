// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "builtins.h"
#include "builtin_types.h"
#include "cpp_interface.h"
#include "runtime.h"
#include "set.h"
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
    void* alloc(Term* term) {
        return NULL;
    }

    void dealloc(void* data) { }

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


struct CompoundValue
{
    int signature;
    Branch branch;
    ReferenceList fields;

    // Member functions
    CompoundValue() : signature(COMPOUND_TYPE_SIGNATURE) {}

    Term* appendSlot(Term* type) {
        Term* newTerm = create_value(&branch, type);
        fields.append(newTerm);
        newTerm->stealingOk = false;
        return newTerm;
    }

    // Static functions
    static void* alloc(Term* typeTerm)
    {
        CompoundValue *value = new CompoundValue();

        // create a slot for each field
        Type& type = as_type(typeTerm);
        int numFields = (int) type.fields.size();

        for (int f=0; f < numFields; f++)
            value->appendSlot(type.fields[f].type);

        return value;
    }

    static void dealloc(void* data)
    {
        delete (CompoundValue*) data;
    }

    /*static void hosted_update_owner(Term* term)
    {
        CompoundValue &value = as_compound_value(term);

        for (int f=0; f < value.fields.count(); f++) {
            update_owner(value.fields[f]);
        }
    }*/

    static void create_compound_type(Term* term)
    {
        std::string name = as_string(term->input(0));
        Type& output = as_type(term);

        output.name = name;
        output.alloc = alloc;
        output.dealloc = dealloc;
    }

    static void append_field(Term* term)
    {
        recycle_value(term->input(0), term);
        Type& output = as_type(term);
        as_type(term->input(1));
        Term* fieldType = term->input(1);
        std::string fieldName = as_string(term->input(2));
        output.addField(fieldType, fieldName);
    }

    static void get_field(Term* term)
    {
        CompoundValue &value = as_compound_value(term->input(0));
        std::string fieldName = as_string(term->input(1));
        Type& type = as_type(term->input(0)->type);

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

bool
Set::contains(Term* value)
{
    std::vector<Term*>::iterator it;
    for (it = members.begin(); it != members.end(); ++it) {
        if (values_equal(value, *it))
            return true;
    }
    return false;
}

void
Set::add(Term* value)
{
    if (contains(value))
        return;

    Term* duplicatedValue = create_value(value->type);
    duplicate_value(value, duplicatedValue);
    members.push_back(duplicatedValue);
}

void
Set::remove(Term* value) 
{
    std::vector<Term*>::iterator it;
    for (it = members.begin(); it != members.end(); ++it) {
        if (values_equal(value, *it)) {

            delete *it;
            members.erase(it);
            return;
        }
    }
}

void
Set::clear()
{
    std::vector<Term*>::iterator it;
    for (it = members.begin(); it != members.end(); ++it) {
        delete *it;
    }

    members.clear();
}

void
Set::hosted_add(Term* caller)
{
    recycle_value(caller->input(0), caller);
    Set& set = as<Set>(caller);
    set.add(caller->input(1));
}

void
Set::hosted_remove(Term* caller)
{
    recycle_value(caller->input(0), caller);
    Set& set = as<Set>(caller);
    set.remove(caller->input(1));
}

std::string
Set::to_string(Term* caller)
{
    Set &set = as<Set>(caller);
    std::vector<Term*>::iterator it;
    std::stringstream output;
    bool first = true;
    output << "{";
    for (it = set.members.begin(); it != set.members.end(); ++it) {
        if (!first) output << ", ";
        output << (*it)->toString();
        first = false;
    }
    output << "}";

    return output.str();
}

bool is_compound_value(Term *term)
{
    assert(term != NULL);
    assert(term->value != NULL);
    return ((CompoundValue*) term->value)->signature == COMPOUND_TYPE_SIGNATURE;
}

CompoundValue& as_compound_value(Term *term)
{
    assert(is_compound_value(term));
    return *((CompoundValue*) term->value);
}

Term* get_field(Term *term, std::string const& fieldName)
{
    assert(is_compound_value(term));
    CompoundValue *value = (CompoundValue*) term->value;
    Type& type = as_type(term->type);
    int index = type.findField(fieldName);
    if (index == -1)
        return NULL;
    return value->fields[index];
}

Term* get_field(Term *term, int index)
{
    assert(is_compound_value(term));
    CompoundValue *value = (CompoundValue*) term->value;
    return value->fields[index];
}

void initialize_builtin_types(Branch& kernel)
{
    STRING_TYPE = quick_create_cpp_type<std::string>(kernel, "string");
    as_type(STRING_TYPE).equals = cpp_interface::templated_equals<std::string>;
    as_type(STRING_TYPE).toString = primitives::string_t::to_string;

    INT_TYPE = quick_create_cpp_type<int>(kernel, "int");
    as_type(INT_TYPE).equals = cpp_interface::templated_equals<int>;
    as_type(INT_TYPE).lessThan = cpp_interface::templated_lessThan<int>;
    as_type(INT_TYPE).toString = primitives::int_t::to_string;

    FLOAT_TYPE = quick_create_cpp_type<float>(kernel, "float");
    as_type(FLOAT_TYPE).equals = cpp_interface::templated_equals<float>;
    as_type(FLOAT_TYPE).lessThan = cpp_interface::templated_lessThan<float>;
    as_type(FLOAT_TYPE).toString = primitives::float_t::to_string;

    BOOL_TYPE = quick_create_cpp_type<bool>(kernel, "bool");
    as_type(BOOL_TYPE).equals = cpp_interface::templated_equals<bool>;
    as_type(BOOL_TYPE).toString = primitives::bool_t::to_string;

    ANY_TYPE = create_empty_type(kernel, "any");
    VOID_TYPE = create_empty_type(kernel, "void");
    REFERENCE_TYPE = quick_create_type(kernel, "Reference");
    as_type(REFERENCE_TYPE).alloc = ref_type::alloc;
    as_type(REFERENCE_TYPE).dealloc = ref_type::dealloc;
    as_type(REFERENCE_TYPE).visitPointers = ref_type::visitPointers;
    as_type(REFERENCE_TYPE).remapPointers = ref_type::remapPointers;

    import_c_function(kernel, CompoundValue::create_compound_type, "create-compound-type(string) -> Type");
    import_c_function(kernel, CompoundValue::append_field, "compound-type-append-field(Type,Type,string) -> Type");
    import_c_function(kernel, CompoundValue::get_field, "get-field(any,string) -> any");

    quick_create_cpp_type<Branch>(kernel, "Branch");

    Term* set_type = quick_create_cpp_type<Set>(kernel, "Set");
    as_type(set_type).toString = Set::to_string;

    Term* set_add = import_c_function(kernel, Set::hosted_add,
        "function Set::add(Set, any) -> Set");
    as_type(set_type).addMemberFunction("add", set_add);

    Term* set_remove = import_c_function(kernel, Set::hosted_remove,
        "function Set::remove(Set, any) -> Set");
    as_type(set_type).addMemberFunction("remove", set_remove);
}

} // namespace circa
