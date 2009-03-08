// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "builtins.h"
#include "builtin_types.h"
#include "cpp_importing.h"
#include "runtime.h"
#include "values.h"

#include "builtin_types/map.hpp"

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
    return *((std::string*) term->value);
}

Ref& as_ref(Term* term)
{
    assert_type(term, REF_TYPE);
    return *((Ref*) term->value);
}

void*& as_void_ptr(Term* term)
{
    assert_type(term, VOID_PTR_TYPE);
    return term->value;
}

namespace primitives {
    namespace int_t {
        std::string to_string(Term* term)
        {
            std::stringstream strm;
            strm << as_int(term);
            return strm.str();
        }

        std::string to_source_string(Term* term)
        {
            std::stringstream strm;
            if (term->syntaxHints.integerFormat == TermSyntaxHints::HEX)
                strm << "0x" << std::hex;

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

        std::string to_source_string(Term* term)
        {
            // Figuring out how many decimal places to show is a hard problem.
            // This will need to be revisited.
            std::stringstream strm;
            strm.setf(std::ios::fixed, std::ios::floatfield);
            strm.precision(term->syntaxHints.floatDecimalFigures);
            strm << as_float(term);

            if (term->hasProperty("mutability")
                    && term->property("mutability")->asFloat() == 1.0)
                strm << "?";
            return strm.str();
        }
    }

    namespace string_t {
        std::string to_string(Term* term)
        {
            return as_string(term);
        }

        std::string to_source_string(Term* term)
        {
            return std::string("'") + as_string(term) + "'";
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

    namespace ptr_t {
        std::string get_cpp_type_name(Term* term)
        {
            Type& type = as_type(term);

            if (type.parameters[0] == ANY_TYPE)
                return "void*";
            else
                return type_id_to_cpp(type.parameters[0]) + "*";
        }
    }

} // namespace primitives

namespace set_t {
    bool contains(Branch& branch, Term* value)
    {
        for (int i=0; i < branch.numTerms(); i++) {
            if (values_equal(value, branch[i]))
                return true;
        }
        return false;
    }

    void add(Branch& branch, Term* value)
    {
        if (contains(branch, value))
            return;

        Term* duplicated_value = create_value(&branch, value->type);
        copy_value(value, duplicated_value);
    }

    void hosted_add(Term* caller)
    {
        recycle_value(caller->input(0), caller);
        Branch& branch = as_branch(caller);
        Term* value = caller->input(1);
        add(branch, value);
    }

    void remove(Term* caller)
    {
        recycle_value(caller->input(0), caller);
        Branch& branch = as_branch(caller);
        Term* value = caller->input(1);

        for (int index=0; index < branch.numTerms(); index++) {
            if (values_equal(value, branch[index])) {

                branch.remove(index);
                return;
            }
        }
    }

    std::string to_string(Term* caller)
    {
        Branch &set = as_branch(caller);
        std::stringstream output;
        bool first = true;
        output << "{";
        for (int i=0; i < set.numTerms(); i++) {
            if (!first) output << ", ";
            output << set[i]->toString();
            first = false;
        }
        output << "}";

        return output.str();
    }

} // namespace set_t

namespace list_t {

    void append(Term* caller)
    {
        recycle_value(caller->input(0), caller);
        Branch& branch = as_branch(caller);
        Term* value = caller->input(1);
        Term* duplicated_value = create_value(&branch, value->type);
        copy_value(value, duplicated_value);
    }

} // namespace list_t

void initialize_builtin_types(Branch& kernel)
{
    STRING_TYPE = import_type<std::string>(kernel, "string");
    as_type(STRING_TYPE).equals = cpp_importing::templated_equals<std::string>;
    as_type(STRING_TYPE).toString = primitives::string_t::to_string;
    as_type(STRING_TYPE).toSourceString = primitives::string_t::to_source_string;

    INT_TYPE = import_type<int>(kernel, "int");
    as_type(INT_TYPE).equals = cpp_importing::templated_equals<int>;
    as_type(INT_TYPE).lessThan = cpp_importing::templated_lessThan<int>;
    as_type(INT_TYPE).toString = primitives::int_t::to_string;
    as_type(INT_TYPE).toSourceString = primitives::int_t::to_source_string;

    FLOAT_TYPE = import_type<float>(kernel, "float");
    as_type(FLOAT_TYPE).equals = cpp_importing::templated_equals<float>;
    as_type(FLOAT_TYPE).lessThan = cpp_importing::templated_lessThan<float>;
    as_type(FLOAT_TYPE).toString = primitives::float_t::to_string;
    as_type(FLOAT_TYPE).toSourceString = primitives::float_t::to_source_string;

    BOOL_TYPE = import_type<bool>(kernel, "bool");
    as_type(BOOL_TYPE).equals = cpp_importing::templated_equals<bool>;
    as_type(BOOL_TYPE).toString = primitives::bool_t::to_string;
    as_type(BOOL_TYPE).toSourceString = primitives::bool_t::to_string;

    ANY_TYPE = create_empty_type(kernel, "any");

    VOID_PTR_TYPE = import_type<void*>(kernel, "void_ptr");
    as_type(VOID_PTR_TYPE).getCppTypeName = primitives::ptr_t::get_cpp_type_name;
    as_type(VOID_PTR_TYPE).parameters.append(ANY_TYPE);

    VOID_TYPE = create_empty_type(kernel, "void");
    REF_TYPE = import_type<Ref>(kernel, "Ref");
    as_type(REF_TYPE).remapPointers = Ref::remap_pointers;
    as_type(REF_TYPE).startReferenceIterator = Ref::start_reference_iterator;

    import_type<RefList>(kernel, "Tuple");
    import_type<Map>(kernel, "Map");

    BRANCH_TYPE = create_empty_type(kernel, "Branch");
    as_type(BRANCH_TYPE).makeCompoundType("Branch");
    assert(as_type(BRANCH_TYPE).alloc == Branch::alloc);

    import_member_function(TYPE_TYPE, Type::name_accessor, "name(Type) -> string");

    Term* set_type = create_empty_type(kernel, "Set");
    as_type(set_type).makeCompoundType("Set");
    as_type(set_type).toString = set_t::to_string;
    import_member_function(set_type, set_t::hosted_add, "function add(Set, any) -> Set");
    import_member_function(set_type, set_t::remove, "function remove(Set, any) -> Set");

    Term* list_type = create_empty_type(kernel, "List");
    as_type(list_type).makeCompoundType("List");
    import_member_function(list_type, list_t::append, "function append(List, any) -> List");
}

} // namespace circa
