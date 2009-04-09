// Copyright 2008 Paul Hodge

#include <iostream>
#include <fstream>

#include "common_headers.h"

#include "circa.h"

#include "builtin_types/map.hpp"

namespace circa {

// setup_builtin_functions is defined in setup_builtin_functions.cpp
void setup_builtin_functions(Branch&);

Branch* KERNEL = NULL;

Term* ASSIGN_FUNC = NULL;
Term* ADD_FUNC = NULL;
Term* ANY_TYPE = NULL;
Term* APPLY_FEEDBACK = NULL;
Term* BOOL_TYPE = NULL;
Term* BRANCH_FUNC = NULL;
Term* BRANCH_TYPE = NULL;
Term* CODEUNIT_TYPE = NULL;
Term* COMMENT_FUNC = NULL;
Term* CONSTANT_TRUE = NULL;
Term* CONSTANT_FALSE = NULL;
Term* COPY_FUNC = NULL;
Term* DIV_FUNC = NULL;
Term* FLOAT_TYPE = NULL;
Term* FOR_FUNC = NULL;
Term* FUNCTION_TYPE = NULL;
Term* GET_FIELD_FUNC = NULL;
Term* IF_FUNC = NULL;
Term* INT_TO_FLOAT_FUNC = NULL;
Term* INT_TYPE = NULL;
Term* LIST_TYPE = NULL;
Term* LIST_FUNC = NULL;
Term* MAP_TYPE = NULL;
Term* MULT_FUNC = NULL;
Term* REF_TYPE = NULL;
Term* SET_FIELD_FUNC = NULL;
Term* SUB_FUNC = NULL;
Term* STATEFUL_VALUE_FUNC = NULL;
Term* STRING_TYPE = NULL;
Term* TYPE_TYPE = NULL;
Term* UNKNOWN_FUNCTION = NULL;
Term* UNKNOWN_IDENTIFIER_FUNC = NULL;
Term* UNKNOWN_TYPE_FUNC = NULL;
Term* UNRECOGNIZED_EXPRESSION_FUNC = NULL;
Term* VALUE_FUNCTION_GENERATOR = NULL;
Term* VALUE_FUNCTION_FEEDBACK_ASSIGN = NULL;
Term* VOID_TYPE = NULL;
Term* VOID_PTR_TYPE = NULL;

void empty_evaluate_function(Term*) { }

void value_function_generate_training(Branch& branch, Term* subject, Term* desired)
{
    apply(&branch, ASSIGN_FUNC, RefList(desired, subject));
}

namespace var_function {

    void feedback_assign(Term* caller)
    {
        Term* target = caller->input(0);
        Term* desired = caller->input(1);

        assign_value(desired, target);
    }
}

namespace primitives {
    namespace int_t {

        std::string to_string(Term* term)
        {
            std::stringstream strm;
            if (term->stringPropertyOptional("syntaxHints:integerFormat", "") == "hex")
                strm << "0x" << std::hex;

            strm << as_int(term);
            return strm.str();
        }
    }

    namespace float_t {

        std::string to_string(Term* term)
        {
            // Figuring out how many decimal places to show is a hard problem.
            // This will need to be revisited.
            std::stringstream strm;

            strm.setf(std::ios::fixed, std::ios::floatfield);
            strm.precision(term->floatPropertyOptional("syntaxHints:decimalFigures", 1));
            strm << as_float(term);

            if (term->floatPropertyOptional("mutability", 0.0) > 0.5)
                strm << "?";

            return strm.str();
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
        for (int i=0; i < branch.numTerms(); i++) {
            if (equals(value, branch[i]))
                return true;
        }
        return false;
    }

    void add(Branch& branch, Term* value)
    {
        if (contains(branch, value))
            return;

        Term* duplicated_value = create_value(&branch, value->type);
        assign_value(value, duplicated_value);
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

        for (int index=0; index < branch.numTerms(); index++) {
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
        for (int i=0; i < set.numTerms(); i++) {
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
        for (int i=0; i < branch.numTerms(); i++) {
            if (i > 0) out << ",";
            out << branch[i]->toString();
        }
        out << "]";
        return out.str();
    }

    void append(Term* caller)
    {
        assign_value(caller->input(0), caller);
        Branch& branch = as_branch(caller);
        Term* value = caller->input(1);
        Term* duplicated_value = create_value(&branch, value->type);
        assign_value(value, duplicated_value);
    }

    void count(Term* caller)
    {
        as_int(caller) = as_branch(caller->input(0)).numTerms();
    }

} // namespace list_t

void value_function_generator(Term* caller)
{
    assert(caller->input(0) != NULL);

    Function& output = as_function(caller);
    Type& type = as_type(caller->input(0));
    output.name = "value-" + type.name;
    output.outputType = caller->input(0);
    output.pureFunction = false;
    output.evaluate = empty_evaluate_function;
    output.generateTraining = value_function_generate_training;
    output.feedbackPropogateFunction = VALUE_FUNCTION_FEEDBACK_ASSIGN;
}

void bootstrap_kernel()
{
    // This is a crazy function. We need to create the 5 core functions in our system,
    // all of which need to reference each other or themselves.
    //
    // Here is what we need to create:
    //
    // var-function-generator(Type) -> Function
    //    Given a type, returns a function which is a 'plain value' function. This term
    //    has function const-Function.
    // const-Type() -> Type
    //    Function which returns a Type value. This is created by var-function-generator
    // const-Function() -> Function
    //    Function which returns a Function value. This is created by var-function-generator.
    // Type Function
    //    Stores the Type object for functions. This term has function const-Type
    // Type Type
    //    Stores the Type object for types. This term has function const-Type

    KERNEL = new Branch();

    // Create var-function-generator function
    VALUE_FUNCTION_GENERATOR = new Term();
    VALUE_FUNCTION_GENERATOR->owningBranch = KERNEL;
    VALUE_FUNCTION_GENERATOR->value = new Function();
    as_function(VALUE_FUNCTION_GENERATOR).name = "value-function-generator";
    as_function(VALUE_FUNCTION_GENERATOR).pureFunction = true;
    as_function(VALUE_FUNCTION_GENERATOR).evaluate = value_function_generator;
    KERNEL->bindName(VALUE_FUNCTION_GENERATOR, "value-function-generator");

    // Create const-Type function
    Term* constTypeFunc = new Term();
    constTypeFunc->owningBranch = KERNEL;
    constTypeFunc->function = VALUE_FUNCTION_GENERATOR;
    constTypeFunc->value = new Function();
    as_function(constTypeFunc).name = "const-Type";
    as_function(constTypeFunc).pureFunction = true;

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
    as_function(VALUE_FUNCTION_GENERATOR).inputTypes.setAt(0, TYPE_TYPE);
    as_function(constTypeFunc).outputType = TYPE_TYPE;

    // Create const-Function function
    Term* constFuncFunc = new Term();
    constFuncFunc->owningBranch = KERNEL;
    constFuncFunc->function = VALUE_FUNCTION_GENERATOR;
    constFuncFunc->value = new Function();
    as_function(constFuncFunc).name = "const-Function";
    as_function(constFuncFunc).pureFunction = true;
    KERNEL->bindName(constFuncFunc, "const-Function");

    // Implant const-Function
    VALUE_FUNCTION_GENERATOR->function = constFuncFunc;

    // Create Function type
    FUNCTION_TYPE = new Term();
    FUNCTION_TYPE->owningBranch = KERNEL;
    FUNCTION_TYPE->function = constTypeFunc;
    FUNCTION_TYPE->type = TYPE_TYPE;
    FUNCTION_TYPE->value = alloc_from_type(TYPE_TYPE);
    as_type(FUNCTION_TYPE).name = "Function";
    as_type(FUNCTION_TYPE).alloc = cpp_importing::templated_alloc<Function>;
    as_type(FUNCTION_TYPE).dealloc = cpp_importing::templated_dealloc<Function>;
    as_type(FUNCTION_TYPE).assign = Function::assign;
    as_type(FUNCTION_TYPE).remapPointers = Function::remapPointers;
    as_type(FUNCTION_TYPE).toSourceString = Function::functionToSourceString;
    KERNEL->bindName(FUNCTION_TYPE, "Function");

    // Implant Function type
    set_input(VALUE_FUNCTION_GENERATOR, 0, TYPE_TYPE);
    set_input(constFuncFunc, 0, FUNCTION_TYPE);
    VALUE_FUNCTION_GENERATOR->type = FUNCTION_TYPE;
    constFuncFunc->type = FUNCTION_TYPE;
    constTypeFunc->type = FUNCTION_TYPE;
    as_function(VALUE_FUNCTION_GENERATOR).outputType = FUNCTION_TYPE;
    as_function(constFuncFunc).outputType = FUNCTION_TYPE;

    // Don't let these terms get updated
    VALUE_FUNCTION_GENERATOR->needsUpdate = false;
    constFuncFunc->needsUpdate = false;
    constTypeFunc->needsUpdate = false;
    FUNCTION_TYPE->needsUpdate = false;
    TYPE_TYPE->needsUpdate = false;
}

void initialize_builtin_types(Branch& kernel)
{
    STRING_TYPE = import_type<std::string>(kernel, "string");
    as_type(STRING_TYPE).equals = cpp_importing::templated_equals<std::string>;
    as_type(STRING_TYPE).lessThan = cpp_importing::templated_lessThan<std::string>;
    as_type(STRING_TYPE).toString = primitives::string_t::to_string;
    as_type(STRING_TYPE).toSourceString = primitives::string_t::to_string;

    INT_TYPE = import_type<int>(kernel, "int");
    as_type(INT_TYPE).equals = cpp_importing::templated_equals<int>;
    as_type(INT_TYPE).lessThan = cpp_importing::templated_lessThan<int>;
    as_type(INT_TYPE).toString = primitives::int_t::to_string;
    as_type(INT_TYPE).toSourceString = primitives::int_t::to_string;

    FLOAT_TYPE = import_type<float>(kernel, "float");
    as_type(FLOAT_TYPE).equals = cpp_importing::templated_equals<float>;
    as_type(FLOAT_TYPE).lessThan = cpp_importing::templated_lessThan<float>;
    as_type(FLOAT_TYPE).toString = primitives::float_t::to_string;
    as_type(FLOAT_TYPE).toSourceString = primitives::float_t::to_string;

    BOOL_TYPE = import_type<bool>(kernel, "bool");
    as_type(BOOL_TYPE).equals = cpp_importing::templated_equals<bool>;
    as_type(BOOL_TYPE).toString = primitives::bool_t::to_string;
    as_type(BOOL_TYPE).toSourceString = primitives::bool_t::to_string;

    ANY_TYPE = create_empty_type(kernel, "any");

    VOID_PTR_TYPE = import_type<void*>(kernel, "void_ptr");
    as_type(VOID_PTR_TYPE).parameters.append(ANY_TYPE);

    VOID_TYPE = create_empty_type(kernel, "void");
    REF_TYPE = import_type<Ref>(kernel, "Ref");
    as_type(REF_TYPE).remapPointers = Ref::remap_pointers;

    import_type<RefList>(kernel, "Tuple");
    import_type<Map>(kernel, "Map");

    BRANCH_TYPE = create_compound_type(kernel, "Branch");
    assert(as_type(BRANCH_TYPE).alloc == Branch::alloc);
    import_member_function(BRANCH_TYPE, list_t::append, "function append(Branch, any) -> Branch");

    import_member_function(TYPE_TYPE, Type::name_accessor, "name(Type) -> string");

    Term* set_type = create_compound_type(kernel, "Set");
    as_type(set_type).toString = set_t::to_string;
    import_member_function(set_type, set_t::hosted_add, "function add(Set, any) -> Set");
    import_member_function(set_type, set_t::remove, "function remove(Set, any) -> Set");

    LIST_TYPE = create_compound_type(kernel, "List");
    as_type(LIST_TYPE).toString = list_t::to_string;
    as_type(LIST_TYPE).toSourceString = list_t::to_string;
    import_member_function(LIST_TYPE, list_t::append, "function append(List, any) -> List");
    import_member_function(LIST_TYPE, list_t::count, "function count(List) -> int");
}

void initialize_builtin_functions(Branch& kernel)
{
    setup_builtin_functions(kernel);

    VALUE_FUNCTION_FEEDBACK_ASSIGN = import_function(kernel,
        var_function::feedback_assign, "var-function-feedback-assign(any,any)");

    as_function(get_value_function(INT_TYPE)).feedbackPropogateFunction = 
        VALUE_FUNCTION_FEEDBACK_ASSIGN;
    as_function(get_value_function(FLOAT_TYPE)).feedbackPropogateFunction = 
        VALUE_FUNCTION_FEEDBACK_ASSIGN;
    as_function(get_value_function(BOOL_TYPE)).feedbackPropogateFunction = 
        VALUE_FUNCTION_FEEDBACK_ASSIGN;
    as_function(get_value_function(STRING_TYPE)).feedbackPropogateFunction = 
        VALUE_FUNCTION_FEEDBACK_ASSIGN;
}

void initialize_constants(Branch& kernel)
{
    CONSTANT_TRUE = apply(&kernel, BOOL_TYPE, RefList());
    as_bool(CONSTANT_TRUE) = true;
    CONSTANT_TRUE->stealingOk = false;
    KERNEL->bindName(CONSTANT_TRUE, "true");

    CONSTANT_FALSE = apply(&kernel, BOOL_TYPE, RefList());
    as_bool(CONSTANT_FALSE) = false;
    CONSTANT_FALSE->stealingOk = false;
    KERNEL->bindName(CONSTANT_FALSE, "false");

    float_value(&kernel, 3.141592654, "PI");
}

void initialize()
{
    bootstrap_kernel();
    initialize_builtin_types(*KERNEL);
    initialize_builtin_functions(*KERNEL);
    initialize_constants(*KERNEL);
}

void shutdown()
{
    delete KERNEL;
    KERNEL = NULL;
}

} // namespace circa
