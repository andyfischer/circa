// Copyright 2008 Andrew Fischer

#include "builtins.h"
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
    feedbackAccumulationFunction(NULL),
    feedbackPropogateFunction(NULL),
    generateCppFunction(NULL),
    printCircaSourceFunction(NULL)
{
}

Term*
Function::inputType(int index)
{
    if (variableArgs)
        return inputTypes[0];
    else
        return inputTypes[index];
}

Function::InputProperties&
Function::getInputProperties(unsigned int index)
{
    assert(index < inputTypes.count());

    // check to grow inputProperties
    while ((index+1) > inputProperties.size()) {
        inputProperties.push_back(InputProperties());
    }

    return inputProperties[index];
}

void Function::setInputMeta(int index, bool value)
{
    getInputProperties(index).meta = value;
}
void Function::setInputModified(int index, bool value)
{
    getInputProperties(index).modified = value;
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

void Function::duplicate(Term* source, Term* dest)
{
    assert(dest->value == NULL);
    dest->value = new Function();
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

    if (branch.containsName(OUTPUT_PLACEHOLDER_NAME)) {
        Term* outputPlaceholder = branch[OUTPUT_PLACEHOLDER_NAME];
        recycle_value(outputPlaceholder, caller);
    }
}

} // namespace circa
