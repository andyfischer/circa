// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {

Function::Function()
  : outputType(NULL),
    stateType(NULL),
    pureFunction(false),
    hasSideEffects(false),
    variableArgs(false),
    evaluate(NULL),
    specializeType(NULL),
    startControlFlowIterator(NULL),
    generateTraining(NULL),
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

void Function::appendInput(Term* type, std::string const& name)
{
    inputTypes.append(type);
    getInputProperties(inputTypes.count()-1).name = name;
    create_value(&subroutineBranch, type, name);
}

Function::InputProperties&
Function::getInputProperties(unsigned int index)
{
    assert(variableArgs || index < inputTypes.count());

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

void Function::copyExceptBranch(Term* sourceTerm, Term* destTerm)
{
    assert(is_function(sourceTerm));
    assert(is_function(destTerm));

    Function &source = as_function(sourceTerm);
    Function &dest = as_function(destTerm);

    dest = Function();

#define copy_field(f) dest.f = source.f

    copy_field(inputTypes);
    copy_field(inputProperties);
    copy_field(outputType);
    copy_field(stateType);
    copy_field(pureFunction);
    copy_field(hasSideEffects);
    copy_field(variableArgs);
    copy_field(name);
    copy_field(evaluate);
    copy_field(startControlFlowIterator);
    copy_field(feedbackAccumulationFunction);
    copy_field(feedbackPropogateFunction);
    copy_field(generateCppFunction);
    copy_field(printCircaSourceFunction);

#undef copy_field
}

void Function::copy(Term* sourceTerm, Term* destTerm)
{
    Function::copyExceptBranch(sourceTerm, destTerm);
    Function &source = as_function(sourceTerm);
    Function &dest = as_function(destTerm);
    duplicate_branch(source.subroutineBranch, dest.subroutineBranch);
}

void Function::remapPointers(Term* term, ReferenceMap const& map)
{
    Function &func = as_function(term);
    func.inputTypes.remapPointers(map);
    func.outputType = map.getRemapped(func.outputType);
    func.stateType = map.getRemapped(func.stateType);
    func.subroutineBranch.remapPointers(map);
}

void initialize_as_subroutine(Function& func)
{
    func.evaluate = Function::subroutine_call_evaluate;
    func.stateType = BRANCH_TYPE;
}

std::string get_placeholder_name_for_index(int index)
{
    std::stringstream sstream;
    sstream << INPUT_PLACEHOLDER_PREFIX << index;
    return sstream.str();
}

void initialize_subroutine_call(Term* term)
{
    Function &def = as_function(term->function);

    as_branch(term->state).clear();
    duplicate_branch(def.subroutineBranch, as_branch(term->state));
}

void
Function::subroutine_call_evaluate(Term* caller)
{
    Branch &branch = as_branch(caller->state);

    if (branch.numTerms() == 0)
        initialize_subroutine_call(caller);

    Function &sub = as_function(caller->function);

    if (sub.inputTypes.count() != caller->inputs.count()) {
        std::stringstream msg;
        msg << "Wrong number of inputs, expected: " << sub.inputTypes.count()
            << ", found: " << caller->inputs.count();
        error_occured(caller, msg.str());
        return;
    }

    // Implant inputs
    for (unsigned int input=0; input < sub.inputTypes.count(); input++) {

        std::string inputName = sub.getInputProperties(input).name;

        Term* inputTerm = branch[inputName];

        assert(inputTerm != NULL);

        recycle_value(caller->inputs[input], inputTerm);
    }

    evaluate_branch(branch);

    // Copy output
    if (branch.contains(OUTPUT_PLACEHOLDER_NAME)) {
        Term* outputPlaceholder = branch[OUTPUT_PLACEHOLDER_NAME];
        assert(outputPlaceholder != NULL);
        recycle_value(outputPlaceholder, caller);
    }
}

Term* create_empty_function(Branch& branch, std::string const& header)
{
    return parser::compile(branch, parser::function_from_header, header);
}

Branch& get_subroutine_branch(Term* term)
{
    return as_function(term).subroutineBranch;
}

} // namespace circa
