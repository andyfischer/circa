// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include <iostream>
#include <fstream>

#include "circa.h"

namespace circa {

// setup_builtin_functions is defined in setup_builtin_functions.cpp
void setup_builtin_functions(Branch&);

Branch* KERNEL = NULL;

Term* ASSIGN_FUNC = NULL;
Term* ADD_FUNC = NULL;
Term* ANNOTATE_TYPE_FUNC = NULL;
Term* ANY_TYPE = NULL;
Term* APPLY_FEEDBACK = NULL;
Term* AVERAGE_FUNC = NULL;
Term* BOOL_TYPE = NULL;
Term* BRANCH_FUNC = NULL;
Term* BRANCH_TYPE = NULL;
Term* CODEUNIT_TYPE = NULL;
Term* COMMENT_FUNC = NULL;
Term* CONSTANT_TRUE = NULL;
Term* CONSTANT_FALSE = NULL;
Term* COPY_FUNC = NULL;
Term* DESIRED_VALUE_FEEDBACK = NULL;
Term* DIV_FUNC = NULL;
Term* FEEDBACK_FUNC = NULL;
Term* FEEDBACK_TYPE = NULL;
Term* FLOAT_TYPE = NULL;
Term* FOR_FUNC = NULL;
Term* FUNCTION_TYPE = NULL;
Term* GET_FIELD_FUNC = NULL;
Term* GET_FIELD_BY_NAME_FUNC = NULL;
Term* IF_FUNC = NULL;
Term* IF_EXPR_FUNC = NULL;
Term* INT_TYPE = NULL;
Term* INPUT_PLACEHOLDER_FUNC = NULL;
Term* LIST_TYPE = NULL;
Term* LIST_FUNC = NULL;
Term* MAP_TYPE = NULL;
Term* MULT_FUNC = NULL;
Term* NOT_FUNC = NULL;
Term* ONE_TIME_ASSIGN_FUNC = NULL;
Term* REF_TYPE = NULL;
Term* SET_FIELD_FUNC = NULL;
Term* SET_FIELD_BY_NAME_FUNC = NULL;
Term* SUB_FUNC = NULL;
Term* SUBROUTINE_TYPE = NULL;
Term* STRING_TYPE = NULL;
Term* TYPE_TYPE = NULL;
Term* UNKNOWN_FUNCTION = NULL;
Term* UNKNOWN_IDENTIFIER_FUNC = NULL;
Term* UNKNOWN_TYPE_FUNC = NULL;
Term* UNRECOGNIZED_EXPRESSION_FUNC = NULL;
Term* VALUE_FUNC = NULL;
Term* VOID_TYPE = NULL;
Term* VOID_PTR_TYPE = NULL;

void empty_evaluate_function(Term*) { }

namespace primitives {
    namespace int_t {

        std::string to_string(Term* term)
        {
            std::stringstream strm;
            if (term->stringPropOptional("syntaxHints:integerFormat", "") == "hex")
                strm << "0x" << std::hex;

            strm << as_int(term);
            return strm.str();
        }
    }

    namespace float_t {

        void assign(Term* source, Term* dest)
        {
            // Allow coercion
            as_float(dest) = to_float(source);
        }

        std::string to_string(Term* term)
        {
            // Correctly formatting floats is a tricky problem.

            // First, check if we know how the user typed this number. If this value
            // still has the exact same value, then use the original formatting.
            if (term->hasProperty("float:original-format")) {
                std::string& originalFormat = term->stringProp("float:original-format");
                float actual = as_float(term);
                float original = atof(originalFormat.c_str());
                if (actual == original) {
                    return originalFormat;
                }
            }

            // Otherwise, format the current value with naive formatting
            std::stringstream strm;
            strm << as_float(term);

            if (term->floatPropOptional("mutability", 0.0) > 0.5)
                strm << "?";

            std::string result = strm.str();

            // Check this string and make sure there is a decimal point. If not, append one.
            bool decimalFound = false;
            for (unsigned i=0; i < result.length(); i++)
                if (result[i] == '.')
                    decimalFound = true;

            if (!decimalFound)
                return result + ".0";
            else
                return result;
        }
    }

    namespace string_t {

        std::string to_string(Term* term)
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

} // namespace primitives

namespace set_t {
    bool contains(Branch& branch, Term* value)
    {
        for (int i=0; i < branch.length(); i++) {
            if (equals(value, branch[i]))
                return true;
        }
        return false;
    }

    void add(Branch& branch, Term* value)
    {
        if (contains(branch, value))
            return;

        create_duplicate(&branch, value);
    }

    void hosted_add(Term* caller)
    {
        assign_value(caller->input(0), caller);
        Branch& branch = as_branch(caller);
        Term* value = caller->input(1);
        add(branch, value);
    }

    void remove(Term* caller)
    {
        assign_value(caller->input(0), caller);
        Branch& branch = as_branch(caller);
        Term* value = caller->input(1);

        for (int index=0; index < branch.length(); index++) {
            if (equals(value, branch[index])) {

                branch.remove(index);
                return;
            }
        }
    }

    std::string to_string(Term* caller)
    {
        Branch &set = as_branch(caller);
        std::stringstream output;
        output << "{";
        for (int i=0; i < set.length(); i++) {
            if (i > 0) output << ", ";
            output << set[i]->toString();
        }
        output << "}";

        return output.str();
    }

} // namespace set_t

namespace list_t {

    std::string to_string(Term* caller)
    {
        std::stringstream out;
        out << "[";
        Branch& branch = as_branch(caller);
        for (int i=0; i < branch.length(); i++) {
            if (i > 0) out << ",";
            out << branch[i]->toString();
        }
        out << "]";
        return out.str();
    }

    void append(Branch& branch, Term* value)
    {
        Term* duplicated_value = create_value(&branch, value->type);
        assign_value(value, duplicated_value);
    }

    void append(Term* caller)
    {
        assign_value(caller->input(0), caller);
        Branch& branch = as_branch(caller);
        Term* value = caller->input(1);
        append(branch, value);
    }

    void count(Term* caller)
    {
        as_int(caller) = as_branch(caller->input(0)).length();
    }

} // namespace list_t

namespace any_t {
    std::string to_string(Term* caller)
    {
        return "<any>";
    }
} // namespace any_t

void bootstrap_kernel()
{
    // Here is what we need to create:
    //
    // value() -> any
    //    Function used for all values
    // const-Type() -> Type
    //    Function which returns a Type value. This is created by var-function-generator
    // const-Function() -> Function
    //    Function which returns a Function value. This is created by var-function-generator.
    // Type Function
    //    Stores the Type object for functions. This term has function const-Type
    // Type Type
    //    Stores the Type object for types. This term has function const-Type

    KERNEL = new Branch();

    // Create value function
    VALUE_FUNC = new Term();
    VALUE_FUNC->owningBranch = KERNEL;
    Function* valueFunc = new Function();
    VALUE_FUNC->value = valueFunc;
    valueFunc->name = "value";
    valueFunc->pureFunction = true;
    valueFunc->evaluate = empty_evaluate_function;
    KERNEL->bindName(VALUE_FUNC, "value");

    // Create const-Type function
    Term* constTypeFunc = new Term();
    constTypeFunc->owningBranch = KERNEL;
    constTypeFunc->function = VALUE_FUNC;
    Function* constTypeFuncValue = new Function();
    constTypeFunc->value = constTypeFuncValue;
    constTypeFuncValue->name = "const-Type";
    constTypeFuncValue->pureFunction = true;

    // Create Type type
    TYPE_TYPE = new Term();
    TYPE_TYPE->owningBranch = KERNEL;
    TYPE_TYPE->function = constTypeFunc;
    TYPE_TYPE->type = TYPE_TYPE;
    Type* typeType = new Type();
    TYPE_TYPE->value = typeType;
    typeType->name = "Type";
    typeType->alloc = Type::type_alloc;
    typeType->dealloc = Type::type_dealloc;
    typeType->assign = Type::type_assign;
    typeType->remapPointers = Type::typeRemapPointers;
    typeType->toString = Type::type_to_string;
    KERNEL->bindName(TYPE_TYPE, "Type");

    // Implant the Type type
    set_input(constTypeFunc, 0, TYPE_TYPE);
    constTypeFuncValue->outputType = TYPE_TYPE;

    // Create const-Function function
    Term* constFuncFunc = new Term();
    constFuncFunc->owningBranch = KERNEL;
    constFuncFunc->function = VALUE_FUNC;
    Function* constFuncFuncValue = new Function();
    constFuncFunc->value = constFuncFuncValue;
    constFuncFuncValue->name = "const-Function";
    constFuncFuncValue->pureFunction = true;
    KERNEL->bindName(constFuncFunc, "const-Function");

    // Create Function type
    FUNCTION_TYPE = new Term();
    FUNCTION_TYPE->owningBranch = KERNEL;
    FUNCTION_TYPE->function = constTypeFunc;
    FUNCTION_TYPE->type = TYPE_TYPE;
    FUNCTION_TYPE->value = new Type();
    as_type(FUNCTION_TYPE).name = "Function";
    as_type(FUNCTION_TYPE).alloc = cpp_importing::templated_alloc<Function>;
    as_type(FUNCTION_TYPE).dealloc = cpp_importing::templated_dealloc<Function>;
    as_type(FUNCTION_TYPE).assign = function_t::assign;
    as_type(FUNCTION_TYPE).remapPointers = function_t::remapPointers;
    as_type(FUNCTION_TYPE).toString = function_t::to_string;
    KERNEL->bindName(FUNCTION_TYPE, "Function");

    // Implant Function type
    set_input(constFuncFunc, 0, FUNCTION_TYPE);
    VALUE_FUNC->type = FUNCTION_TYPE;
    constFuncFunc->type = FUNCTION_TYPE;
    constTypeFunc->type = FUNCTION_TYPE;
    as_function(constFuncFunc).outputType = FUNCTION_TYPE;

    // Create Any type
    ANY_TYPE = new Term();
    ANY_TYPE->owningBranch = KERNEL;
    ANY_TYPE->function = VALUE_FUNC;
    ANY_TYPE->type = TYPE_TYPE;
    ANY_TYPE->value = new Type();
    as_type(ANY_TYPE).name = "any";
    KERNEL->bindName(ANY_TYPE, "any");

    as_function(VALUE_FUNC).outputType = ANY_TYPE;
}

void initialize_builtin_types(Branch& kernel)
{
    STRING_TYPE = import_type<std::string>(kernel, "string");
    as_type(STRING_TYPE).equals = cpp_importing::templated_equals<std::string>;
    as_type(STRING_TYPE).lessThan = cpp_importing::templated_lessThan<std::string>;
    as_type(STRING_TYPE).toString = primitives::string_t::to_string;

    INT_TYPE = import_type<int>(kernel, "int");
    as_type(INT_TYPE).equals = cpp_importing::templated_equals<int>;
    as_type(INT_TYPE).lessThan = cpp_importing::templated_lessThan<int>;
    as_type(INT_TYPE).toString = primitives::int_t::to_string;

    FLOAT_TYPE = import_type<float>(kernel, "float");
    as_type(FLOAT_TYPE).assign = primitives::float_t::assign;
    as_type(FLOAT_TYPE).equals = cpp_importing::templated_equals<float>;
    as_type(FLOAT_TYPE).lessThan = cpp_importing::templated_lessThan<float>;
    as_type(FLOAT_TYPE).toString = primitives::float_t::to_string;

    BOOL_TYPE = import_type<bool>(kernel, "bool");
    as_type(BOOL_TYPE).equals = cpp_importing::templated_equals<bool>;
    as_type(BOOL_TYPE).toString = primitives::bool_t::to_string;

    ANY_TYPE = create_empty_type(kernel, "any");
    as_type(ANY_TYPE).toString = any_t::to_string;

    as_function(VALUE_FUNC).outputType = ANY_TYPE;

    VOID_PTR_TYPE = import_type<void*>(kernel, "void_ptr");
    as_type(VOID_PTR_TYPE).parameters.append(ANY_TYPE);

    VOID_TYPE = create_empty_type(kernel, "void");
    REF_TYPE = import_type<Ref>(kernel, "Ref");
    as_type(REF_TYPE).remapPointers = Ref::remap_pointers;

    import_type<RefList>(kernel, "Tuple");

    BRANCH_TYPE = create_compound_type(kernel, "Branch");
    assert(as_type(BRANCH_TYPE).alloc == Branch::alloc);
    import_member_function(BRANCH_TYPE, list_t::append, "append(Branch, any) : Branch");

    import_member_function(TYPE_TYPE, Type::name_accessor, "name(Type) : string");

    Term* set_type = create_compound_type(kernel, "Set");
    as_type(set_type).toString = set_t::to_string;
    import_member_function(set_type, set_t::hosted_add, "add(Set, any) : Set");
    import_member_function(set_type, set_t::remove, "remove(Set, any) : Set");

    LIST_TYPE = create_compound_type(kernel, "List");
    as_type(LIST_TYPE).toString = list_t::to_string;
    import_member_function(LIST_TYPE, list_t::append, "append(List, any) : List");
    import_member_function(LIST_TYPE, list_t::count, "count(List) : int");

    register_subroutine_type(kernel);
    feedback_register_constants(kernel);
}

void initialize_constants(Branch& kernel)
{
    float_value(&kernel, M_PI, "PI");
    float_value(&kernel, M_PI / 2.0, "HALF_PI");

    as_function(VALUE_FUNC).feedbackFunc = ASSIGN_FUNC;
}

void initialize()
{
    bootstrap_kernel();
    initialize_builtin_types(*KERNEL);
    setup_builtin_functions(*KERNEL);
    initialize_constants(*KERNEL);
}

void shutdown()
{
    delete KERNEL;
    KERNEL = NULL;
}

} // namespace circa
