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
Term* APPLY_FEEDBACK = NULL;
Term* AVERAGE_FUNC = NULL;
Term* BRANCH_FUNC = NULL;
Term* BRANCH_TYPE = NULL;
Term* CODEUNIT_TYPE = NULL;
Term* COMMENT_FUNC = NULL;
Term* CONSTANT_TRUE = NULL;
Term* CONSTANT_FALSE = NULL;
Term* COPY_FUNC = NULL;
Term* DESIRED_VALUE_FEEDBACK = NULL;
Term* DIV_FUNC = NULL;
Term* DO_ONCE_FUNC = NULL;
Term* FEEDBACK_FUNC = NULL;
Term* FEEDBACK_TYPE = NULL;
Term* FOR_FUNC = NULL;
Term* FUNCTION_TYPE = NULL;
Term* GET_INDEX_FUNC = NULL;
Term* GET_FIELD_FUNC = NULL;
Term* IF_FUNC = NULL;
Term* IF_BLOCK_FUNC = NULL;
Term* IF_EXPR_FUNC = NULL;
Term* INPUT_PLACEHOLDER_FUNC = NULL;
Term* LIST_TYPE = NULL;
Term* LIST_FUNC = NULL;
Term* MAP_TYPE = NULL;
Term* MULT_FUNC = NULL;
Term* NEG_FUNC = NULL;
Term* NOT_FUNC = NULL;
Term* ONE_TIME_ASSIGN_FUNC = NULL;
Term* OVERLOADED_FUNCTION_TYPE = NULL;
Term* SET_FIELD_FUNC = NULL;
Term* SET_INDEX_FUNC = NULL;
Term* SUB_FUNC = NULL;
Term* TYPE_TYPE = NULL;
Term* UNKNOWN_FUNCTION = NULL;
Term* UNKNOWN_FIELD_FUNC = NULL;
Term* UNKNOWN_IDENTIFIER_FUNC = NULL;
Term* UNKNOWN_TYPE_FUNC = NULL;
Term* UNRECOGNIZED_EXPRESSION_FUNC = NULL;
Term* VALUE_FUNC = NULL;
Term* VOID_TYPE = NULL;
Term* VOID_PTR_TYPE = NULL;

void empty_evaluate_function(Term*) { }

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

        create_duplicate(branch, value);
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
        Term* duplicated_value = create_value(branch, value->type);
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

namespace dict_t {
    std::string to_string(Branch& branch)
    {
        std::stringstream out;
        out << "{";
        for (int i=0; i < branch.length(); i++) {
            Term* term = branch[i];
            std::string name = term->name;
            if (name == "")
                name = "<anon>";

            if (i != 0)
                out << ", ";

            out << name << ": " << to_string(term);
        }
        out << "}";
        return out.str();
    }
} // namespace dict_t

void bootstrap_kernel()
{
    // Create the very first building blocks. Most of the building functions in Circa
    // require a few kernel terms to already be defined. So in this function, we
    // create these required terms manually.

    KERNEL = new Branch();

    // Create value function
    VALUE_FUNC = new Term();
    VALUE_FUNC->owningBranch = KERNEL;
    KERNEL->bindName(VALUE_FUNC, "value");

    // Create Type type
    TYPE_TYPE = new Term();
    TYPE_TYPE->owningBranch = KERNEL;
    TYPE_TYPE->function = VALUE_FUNC;
    TYPE_TYPE->type = TYPE_TYPE;
    Type* typeType = new Type();
    TYPE_TYPE->value = typeType;
    typeType->name = "Type";
    typeType->alloc = type_t::alloc;
    typeType->dealloc = type_t::dealloc;
    typeType->assign = type_t::assign;
    typeType->remapPointers = type_t::remap_pointers;
    typeType->toString = type_t::to_string;
    KERNEL->bindName(TYPE_TYPE, "Type");

    // Create Any type
    ANY_TYPE = new Term();
    ANY_TYPE->owningBranch = KERNEL;
    ANY_TYPE->function = VALUE_FUNC;
    ANY_TYPE->type = TYPE_TYPE;
    ANY_TYPE->value = new Type();
    as_type(ANY_TYPE).name = "any";
    KERNEL->bindName(ANY_TYPE, "any");

    // Create Function type
    FUNCTION_TYPE = create_compound_type(*KERNEL, "Function");
    as_type(FUNCTION_TYPE).toString = subroutine_t::to_string;

    // Create Branch type
    BRANCH_TYPE = create_compound_type(*KERNEL, "Branch");

    // Initialize Value func
    VALUE_FUNC->type = FUNCTION_TYPE;
    alloc_value(VALUE_FUNC);
}

void initialize_builtin_types(Branch& kernel)
{
    VOID_PTR_TYPE = import_type<void*>(kernel, "void_ptr");
    as_type(VOID_PTR_TYPE).parameters.append(ANY_TYPE);

    import_type<RefList>(kernel, "Tuple");

    import_member_function(BRANCH_TYPE, list_t::append, "append(Branch, any) : Branch");

    import_member_function(TYPE_TYPE, type_t::name_accessor, "name(Type) : string");

    Term* set_type = create_compound_type(kernel, "Set");
    as_type(set_type).toString = set_t::to_string;
    import_member_function(set_type, set_t::hosted_add, "add(Set, any) : Set");
    import_member_function(set_type, set_t::remove, "remove(Set, any) : Set");

    as_type(LIST_TYPE).toString = list_t::to_string;
    import_member_function(LIST_TYPE, list_t::append, "append(List, any) : List");
    import_member_function(LIST_TYPE, list_t::count, "count(List) : int");

    OVERLOADED_FUNCTION_TYPE = create_compound_type(kernel, "OverloadedFunction");

    feedback_register_constants(kernel);
}

void post_setup_builtin_functions(Branch& kernel)
{
    Branch& add_overloads = as_branch(ADD_FUNC);
    Term* add_v = create_duplicate(add_overloads, kernel["vectorize_vv"]);
    create_ref(function_get_parameters(add_v), ADD_FUNC);
    rename(add_v, "add_v");
    kernel.bindName(add_v, "add_v");

    Branch& sub_overloads = as_branch(SUB_FUNC);
    Term* sub_v = create_duplicate(sub_overloads, kernel["vectorize_vv"]);
    create_ref(function_get_parameters(sub_v), SUB_FUNC);
    rename(sub_v, "sub_v");
    kernel.bindName(sub_v, "sub_v");

    Branch& mult_overloads = as_branch(MULT_FUNC);
    Term* mult_s = create_duplicate(mult_overloads, kernel["vectorize_vs"]);
    create_ref(function_get_parameters(mult_s), MULT_FUNC);
    rename(mult_s, "mult_s");
    kernel.bindName(mult_s, "mult_s");

    function_get_feedback_func(VALUE_FUNC) = ASSIGN_FUNC;
}

void initialize()
{
    bootstrap_kernel();
    initialize_primitive_types(*KERNEL);

    // Initialize data for Value function
    initialize_function_data(VALUE_FUNC);
    
    // Define output type
    create_value(as_branch(VALUE_FUNC), ANY_TYPE);

    // Declare input_placeholder first because it's used while compiling functions
    //INPUT_PLACEHOLDER_FUNC = create_value(KERNEL, FUNCTION_TYPE, "input_placeholder");
    INPUT_PLACEHOLDER_FUNC = import_function(*KERNEL, empty_evaluate_function,
            "input_placeholder() : any");
    //function_get_input_placeholder(INPUT_PLACEHOLDER_FUNC, 0)->function = INPUT_PLACEHOLDER_FUNC;
    

    function_get_output_type(VALUE_FUNC) = ANY_TYPE;

    initialize_builtin_types(*KERNEL);
    setup_builtin_functions(*KERNEL);
    post_setup_builtin_functions(*KERNEL);
}

void shutdown()
{
    delete KERNEL;
    KERNEL = NULL;
}

} // namespace circa
