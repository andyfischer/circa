// Copyright 2008 Paul Hodge

#include <iostream>
#include <fstream>

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "cpp_interface.h"
#include "errors.h"
#include "feedback.h"
#include "function.h"
#include "importing.h"
#include "list.h"
#include "runtime.h"
#include "tokenizer.h"
#include "token_stream.h"
#include "term.h"
#include "type.h"
#include "values.h"

namespace circa {

Branch* KERNEL = NULL;
Term* VAR_FUNCTION_GENERATOR = NULL;
Term* VAR_FUNCTION_FEEDBACK_ASSIGN = NULL;
Term* INT_TYPE = NULL;
Term* FLOAT_TYPE = NULL;
Term* BOOL_TYPE = NULL;
Term* STRING_TYPE = NULL;
Term* TYPE_TYPE = NULL;
Term* FUNCTION_TYPE = NULL;
Term* CODEUNIT_TYPE = NULL;
Term* BRANCH_TYPE = NULL;
Term* ANY_TYPE = NULL;
Term* VOID_TYPE = NULL;
Term* REFERENCE_TYPE = NULL;
Term* LIST_TYPE = NULL;
Term* MAP_TYPE = NULL;
Term* VAR_INT = NULL;
Term* VAR_FLOAT = NULL;
Term* VAR_STRING = NULL;
Term* VAR_BOOL = NULL;
Term* CONSTANT_0 = NULL;
Term* CONSTANT_1 = NULL;
Term* CONSTANT_2 = NULL;
Term* CONSTANT_TRUE = NULL;
Term* CONSTANT_FALSE = NULL;
Term* UNKNOWN_FUNCTION = NULL;
Term* APPLY_FEEDBACK = NULL;
Term* ADD_FUNC = NULL;
Term* MULT_FUNC = NULL;

void empty_evaluate_function(Term*) { }
void empty_alloc_function(Term*) { }

void var_function_generator(Term* caller)
{
    assert(caller->inputs[0] != NULL);

    Function& output = as_function(caller);
    Type& type = as_type(caller->inputs[0]);
    output.name = "var-" + type.name;
    output.outputType = caller->inputs[0];
    output.evaluate = empty_evaluate_function;
    output.feedbackPropogateFunction = VAR_FUNCTION_FEEDBACK_ASSIGN;
}

Term* get_global(std::string name)
{
    if (KERNEL->containsName(name))
        return KERNEL->getNamed(name);

    return NULL;
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
    //    Type is a type, this term stores the Type object for types. This term has
    //    function const-Type

    KERNEL = new Branch();

    // Create var-function-generator function
    VAR_FUNCTION_GENERATOR = new Term();
    Function::alloc(VAR_FUNCTION_GENERATOR);
    as_function(VAR_FUNCTION_GENERATOR).name = "var-function-generator";
    as_function(VAR_FUNCTION_GENERATOR).pureFunction = true;
    as_function(VAR_FUNCTION_GENERATOR).evaluate = var_function_generator;
    KERNEL->bindName(VAR_FUNCTION_GENERATOR, "var-function-generator");

    // Create const-Type function
    Term* constTypeFunc = new Term();
    constTypeFunc->function = VAR_FUNCTION_GENERATOR;
    Function::alloc(constTypeFunc);
    as_function(constTypeFunc).name = "const-Type";
    as_function(constTypeFunc).pureFunction = true;

    // Create Type type
    Term* typeType = new Term();
    TYPE_TYPE = typeType;
    typeType->function = constTypeFunc;
    typeType->type = typeType;
    initialize_type_type(typeType);
    as_type(TYPE_TYPE).remapPointers = Type::typeRemapPointers;
    as_type(TYPE_TYPE).visitPointers = Type::typeVisitPointers;
    KERNEL->bindName(typeType, "Type");

    // Implant the Type type
    set_input(constTypeFunc, 0, typeType);
    as_function(VAR_FUNCTION_GENERATOR).inputTypes.setAt(0, typeType);
    as_function(constTypeFunc).outputType = typeType;

    // Create const-Function function
    Term* constFuncFunc = new Term();
    constFuncFunc->function = VAR_FUNCTION_GENERATOR;
    Function::alloc(constFuncFunc);
    as_function(constFuncFunc).name = "const-Function";
    as_function(constFuncFunc).pureFunction = true;
    KERNEL->bindName(constFuncFunc, "const-Function");

    // Implant const-Function
    VAR_FUNCTION_GENERATOR->function = constFuncFunc;

    // Create Function type
    Term* functionType = new Term();
    FUNCTION_TYPE = functionType;
    functionType->function = constTypeFunc;
    functionType->type = typeType;
    as_type(typeType).alloc(functionType);
    as_type(functionType).name = "Function";
    as_type(functionType).alloc = Function::alloc;
    as_type(functionType).duplicate = Function::duplicate;
    as_type(functionType).dealloc = Function::dealloc;
    as_type(functionType).remapPointers = Function::remapPointers;
    as_type(functionType).visitPointers = Function::visitPointers;
    KERNEL->bindName(functionType, "Function");

    // Implant Function type
    set_input(VAR_FUNCTION_GENERATOR, 0, typeType);
    set_input(constFuncFunc, 0, functionType);
    VAR_FUNCTION_GENERATOR->type = functionType;
    constFuncFunc->type = functionType;
    constTypeFunc->type = functionType;
    as_function(VAR_FUNCTION_GENERATOR).outputType = functionType;
    as_function(constFuncFunc).outputType = functionType;

    // Don't let these terms get updated
    VAR_FUNCTION_GENERATOR->needsUpdate = false;
    constFuncFunc->needsUpdate = false;
    constTypeFunc->needsUpdate = false;
    functionType->needsUpdate = false;
    typeType->needsUpdate = false;
}

void print__evaluate(Term* caller)
{
    std::cout << as_string(caller->inputs[0]) << std::endl;
}

namespace add_function {

    void evaluate(Term* caller)
    {
        as_float(caller) = as_float(caller->inputs[0]) + as_float(caller->inputs[1]);
    }
    void feedback_propogate(Term* caller)
    {
        Term* target = caller->inputs[0];
        Term* desired = caller->inputs[1];
        Branch& myBranch = as_branch(caller->state);
        myBranch.clear();

        // for now, send 1/n of the delta to each input
        float delta = as_float(desired) - as_float(target);

        int numInputs = target->inputs.count();

        if (numInputs == 0)
            return;

        for (int i=0; i < numInputs; i++) {
            float inputDelta = delta / numInputs;

            Term* input = target->inputs[i];

            apply_function(myBranch, APPLY_FEEDBACK,
                ReferenceList(input, float_var(myBranch, as_float(input) + inputDelta)));
        }

        evaluate_branch(myBranch);
    }
    static void setup(Branch* kernel)
    {
        ADD_FUNC = quick_create_function(kernel, "add", evaluate,
            ReferenceList(FLOAT_TYPE, FLOAT_TYPE), FLOAT_TYPE);

        Term* fp_func =
            quick_create_function(kernel, "add-feedback-propogate",
                feedback_propogate,
                ReferenceList(ANY_TYPE, ANY_TYPE), VOID_TYPE);
        as_function(fp_func).stateType = BRANCH_TYPE;

        as_function(ADD_FUNC).feedbackPropogateFunction = fp_func;
    }
}

void mult__evaluate(Term* caller)
{
    as_float(caller) = as_float(caller->inputs[0]) * as_float(caller->inputs[1]);
}

void string_concat__evaluate(Term* caller)
{
    as_string(caller) = as_string(caller->inputs[0]) + as_string(caller->inputs[1]);
}

void if_expr__evaluate(Term* caller)
{
    int index = as_bool(caller->inputs[0]) ? 1 : 2;
    Term *result = caller->inputs[index];
    change_type(caller, result->type);
    recycle_value(caller->inputs[index], caller);
}

void range__evaluate(Term* caller)
{
    unsigned int max = as_int(caller->inputs[0]);

    as_list(caller).clear();

    for (unsigned int i=0; i < max; i++) {
        as_list(caller).append(int_var(*caller->owningBranch, i));
    }
}

void list_append__evaluate(Term* caller)
{
    recycle_value(caller->inputs[0], caller);
    as_list(caller).append(caller->inputs[1]);
}

void list_apply__evaluate(Term* caller)
{
    as_function(caller->inputs[0]);
    List& list = as_list(caller->inputs[1]);

    as_list(caller).clear();

    for (int i=0; i < list.count(); i++) {
        Term* result = apply_function(*caller->owningBranch, caller->inputs[0], ReferenceList(list.get(i)));

        result->eval();

        as_list(caller).append(result);
    }
}

void read_text_file__evaluate(Term* caller)
{
    std::string filename = as_string(caller->inputs[0]);
    std::ifstream file;
    file.open(filename.c_str(), std::ios::in);
    std::stringstream contents;
    std::string line;
    bool firstLine = true;
    while (std::getline(file, line)) {
        if (!firstLine)
            contents << "\n";
        contents << line;
        firstLine = false;
    }
    file.close();
    as_string(caller) = contents.str();
}

void write_text_file__evaluate(Term* caller)
{
    std::string filename = as_string(caller->inputs[0]);
    std::string contents = as_string(caller->inputs[1]);
    std::ofstream file;
    file.open(filename.c_str(), std::ios::out);
    file << contents;
    file.close();
}

void to_string__evaluate(Term* caller)
{
    as_string(caller) = caller->inputs[0]->toString();
}

void unknown_function__evaluate(Term* caller)
{
    std::cout << "Warning, calling an unknown function: "
        << as_string(caller->state) << std::endl;
}

namespace var_function {

    void feedback_assign(Term* caller)
    {
        Term* target = caller->inputs[0];
        Term* desired = caller->inputs[1];

        duplicate_value(desired, target);
    }
}

void initialize_constants()
{
    BRANCH_TYPE = quick_create_cpp_type<Branch>(KERNEL, "Branch");
    as_type(BRANCH_TYPE).remapPointers = Branch::hosted_remap_pointers;
    as_type(BRANCH_TYPE).visitPointers = Branch::hosted_visit_pointers;

    LIST_TYPE = quick_create_cpp_type<List>(KERNEL, "List");
    as_type(LIST_TYPE).toString = List__toString;

    VAR_INT = get_var_function(*KERNEL, INT_TYPE);
    VAR_FLOAT = get_var_function(*KERNEL, FLOAT_TYPE);
    VAR_STRING = get_var_function(*KERNEL, STRING_TYPE);
    VAR_BOOL = get_var_function(*KERNEL, BOOL_TYPE);

    CONSTANT_0 = float_var(*KERNEL, 0);
    CONSTANT_1 = float_var(*KERNEL, 1);
    CONSTANT_2 = float_var(*KERNEL, 2);

    CONSTANT_TRUE = apply_function(*KERNEL, BOOL_TYPE, ReferenceList());
    as_bool(CONSTANT_TRUE) = true;
    KERNEL->bindName(CONSTANT_TRUE, "true");
    CONSTANT_FALSE = apply_function(*KERNEL, BOOL_TYPE, ReferenceList());
    as_bool(CONSTANT_FALSE) = false;
    KERNEL->bindName(CONSTANT_FALSE, "false");
}

void initialize_builtin_functions(Branch* kernel)
{
    add_function::setup(kernel);
    MULT_FUNC = quick_create_function(kernel, "mult", mult__evaluate,
            ReferenceList(FLOAT_TYPE, FLOAT_TYPE), FLOAT_TYPE);
    quick_create_function(kernel, "concat", string_concat__evaluate, ReferenceList(STRING_TYPE, STRING_TYPE), STRING_TYPE);
    quick_create_function(kernel, "print", print__evaluate, ReferenceList(STRING_TYPE), VOID_TYPE);
    quick_create_function(kernel, "if-expr", if_expr__evaluate,
        ReferenceList(BOOL_TYPE, ANY_TYPE, ANY_TYPE), ANY_TYPE);
    quick_create_function(kernel, "range", range__evaluate, ReferenceList(INT_TYPE), LIST_TYPE);
    quick_create_function(kernel, "list-append", list_append__evaluate, ReferenceList(LIST_TYPE, ANY_TYPE), LIST_TYPE);
    quick_create_function(kernel, "list-apply", list_apply__evaluate, ReferenceList(FUNCTION_TYPE, LIST_TYPE), LIST_TYPE);
    quick_create_function(kernel, "read-text-file", read_text_file__evaluate,
            ReferenceList(STRING_TYPE), STRING_TYPE);
    quick_create_function(kernel, "write-text-file", write_text_file__evaluate,
            ReferenceList(STRING_TYPE, STRING_TYPE), VOID_TYPE);
    quick_create_function(kernel, "to-string", to_string__evaluate,
        ReferenceList(ANY_TYPE), STRING_TYPE);
    UNKNOWN_FUNCTION = quick_create_function(kernel, "unknown-function",
            unknown_function__evaluate,
            ReferenceList(ANY_TYPE), ANY_TYPE);
    as_function(UNKNOWN_FUNCTION).stateType = STRING_TYPE;

    VAR_FUNCTION_FEEDBACK_ASSIGN = quick_create_function(kernel, "var-function-feedback-assign",
        var_function::feedback_assign,
        ReferenceList(ANY_TYPE, ANY_TYPE), VOID_TYPE);

    as_function(VAR_INT).feedbackPropogateFunction = VAR_FUNCTION_FEEDBACK_ASSIGN;
    as_function(VAR_FLOAT).feedbackPropogateFunction = VAR_FUNCTION_FEEDBACK_ASSIGN;
    as_function(VAR_BOOL).feedbackPropogateFunction = VAR_FUNCTION_FEEDBACK_ASSIGN;
    as_function(VAR_STRING).feedbackPropogateFunction = VAR_FUNCTION_FEEDBACK_ASSIGN;
}

void initialize()
{
    try {
        bootstrap_kernel();
        initialize_primitive_types(KERNEL);
        initialize_constants();
        initialize_compound_types(KERNEL);
        initialize_list_functions(KERNEL);

        // Then everything else:
        initialize_builtin_functions(KERNEL);
        initialize_feedback_functions(*KERNEL);
        initialize_functions(KERNEL);

    } catch (errors::CircaError& e)
    {
        std::cout << "An error occured while initializing." << std::endl;
        std::cout << e.message() << std::endl;
        exit(1);
    }
}

void shutdown()
{
    delete KERNEL;
    KERNEL = NULL;
}

Term* get_var_function(Branch& branch, Term* type)
{
    Term* result = apply_function(branch, VAR_FUNCTION_GENERATOR, ReferenceList(type));
    result->eval();
    return result;
}

bool is_var(Term* term)
{
    return term->function->function == VAR_FUNCTION_GENERATOR;
}

} // namespace circa
