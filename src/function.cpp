// Copyright 2008 Paul Hodge

#include "builtins.h"
#include "errors.h"
#include "function.h"
#include "importing.h"
#include "list.h"
#include "parser.h"
#include "runtime.h"
#include "type.h"
#include "values.h"

namespace circa {

Function::Function()
  : outputType(NULL),
    stateType(NULL),
    pureFunction(false),
    hasSideEffects(false),
    variableArgs(false),
    initialize(NULL),
    evaluate(NULL),
    meta(false),
    feedbackAccumulationFunction(NULL),
    feedbackPropogateFunction(NULL)
{
}

bool is_function(Term* term)
{
    return term->type == FUNCTION_TYPE;
}

Function& as_function(Term* term)
{
    assert_type(term, FUNCTION_TYPE);
    assert(term->value != NULL);
    return *((Function*) term->value);
}

void Function::alloc(Term* caller)
{
    caller->value = new Function();
}

void Function::dealloc(Term* caller)
{
    delete (Function*) caller->value;
}

void Function::duplicate(Term* source, Term* dest)
{
    Function::alloc(dest);
    as_function(dest) = as_function(source);
}

void Function::remapPointers(Term* term, ReferenceMap const& map)
{
    Function &func = as_function(term);
    func.inputTypes.remapPointers(map);
    func.outputType = map.getRemapped(func.outputType);
    func.stateType = map.getRemapped(func.stateType);
    func.subroutineBranch.remapPointers(map);
}

void Function::visitPointers(Term* term, PointerVisitor& visitor)
{
    Function &func = as_function(term);
    func.inputTypes.visitPointers(visitor);
    visitor.visitPointer(func.outputType);
    visitor.visitPointer(func.stateType);
    func.subroutineBranch.visitPointers(visitor);
}

std::string get_placeholder_name_for_index(int index)
{
    std::stringstream sstream;
    sstream << INPUT_PLACEHOLDER_PREFIX << index;
    return sstream.str();
}

void Function::subroutine_create(Term* caller)
{
    // 0: name (string)
    // 1: inputTypes (list of type)
    // 2: outputType (type)

    as_string(caller->inputs[0]);
    as_list(caller->inputs[1]);
    as_type(caller->inputs[2]);

    Function& sub = as_function(caller);
    sub.name = as_string(caller->inputs[0]);
    sub.initialize = Function::call_subroutine__initialize;
    sub.evaluate = Function::call_subroutine;

    // extract references to input types
    {
        Branch workspace;
        Term* list_refs = eval_function(workspace, "get-list-references",
                ReferenceList(caller->inputs[1]));

        if (list_refs->hasError()) {
            error_occured(caller, std::string("get-list-references error: ")
                + list_refs->getError(0));
            return;
        }

        sub.inputTypes = as_list(list_refs).toReferenceList();
    }

    sub.outputType = caller->inputs[2];
    sub.stateType = BRANCH_TYPE;

    // Create input placeholders
    for (unsigned int index=0; index < sub.inputTypes.count(); index++) {
        std::string name = get_placeholder_name_for_index(index);
        Term* placeholder = create_var(&sub.subroutineBranch,
            sub.inputTypes[index]);
        sub.subroutineBranch.bindName(placeholder, name);
    }
}

void
Function::call_subroutine__initialize(Term* caller)
{
}

void
Function::call_subroutine(Term* caller)
{
    Function &sub = as_function(caller->function);

    if (sub.inputTypes.count() != caller->inputs.count()) {
        std::stringstream msg;
        msg << "Wrong number of inputs, expected: " << sub.inputTypes.count()
            << ", found: " << caller->inputs.count();
        error_occured(caller, msg.str());
        return;
    }

    Branch branch;

    duplicate_branch(&sub.subroutineBranch, &branch);

    for (unsigned int input=0; input < sub.inputTypes.count(); input++) {

        Term* inputPlaceholder = branch[get_placeholder_name_for_index(input)];

        recycle_value(caller->inputs[input], inputPlaceholder);
    }
    
    evaluate_branch(branch);

    Term* outputPlaceholder = branch[OUTPUT_PLACEHOLDER_NAME];

    recycle_value(outputPlaceholder, caller);
}

void Function::name_input(Term* caller)
{
    recycle_value(caller->inputs[0], caller);
    int index = as_int(caller->inputs[1]);
    std::string name = as_string(caller->inputs[2]);
    Function& sub = as_function(caller);

    if (index < 0) {
        error_occured(caller, "index must be >= 0");
        return;
    }

    if ((unsigned int) index >= sub.inputTypes.count()) {
        error_occured(caller, "index out of bounds");
        return;
    }

    Term* inputPlaceholder =
        sub.subroutineBranch.getNamed(get_placeholder_name_for_index(index));

    assert(inputPlaceholder != NULL);

    // remove the name on this term, so that this new name will stick
    inputPlaceholder->name = "";
    sub.subroutineBranch.bindName(inputPlaceholder, name);
}

void Function::get_input_name(Term* caller)
{
    Function& sub = as_function(caller->inputs[0]);
    int index = as_int(caller->inputs[1]);

    Term* inputPlaceholder =
        sub.subroutineBranch.getNamed(get_placeholder_name_for_index(index));

    as_string(caller) = inputPlaceholder->name;
}

void Function::subroutine_apply(Term* caller)
{
    recycle_value(caller->inputs[0], caller);
    std::string input = as_string(caller->inputs[1]);

    Function& sub = as_function(caller);

    apply_statement(sub.subroutineBranch, input);
}

void initialize_functions(Branch* kernel)
{
    quick_create_function(kernel, "subroutine-create",
        Function::subroutine_create,
        ReferenceList(STRING_TYPE,LIST_TYPE,TYPE_TYPE),
        FUNCTION_TYPE);

    quick_create_function(kernel,
        "function-name-input", Function::name_input,
        ReferenceList(FUNCTION_TYPE, INT_TYPE, STRING_TYPE), FUNCTION_TYPE);

    quick_create_function(kernel,
        "function-get-input-name", Function::get_input_name,
        ReferenceList(FUNCTION_TYPE, INT_TYPE), STRING_TYPE);

    quick_create_function(kernel,
        "subroutine-apply", Function::subroutine_apply,
        ReferenceList(FUNCTION_TYPE, STRING_TYPE), FUNCTION_TYPE);
}

} // namespace circa
