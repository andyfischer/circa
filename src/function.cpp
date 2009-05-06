// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {

Function::Function()
  : outputType(NULL),
    hiddenStateType(NULL),
    pureFunction(false),
    hasSideEffects(false),
    variableArgs(false),
    evaluate(NULL),
    specializeType(NULL),
    startControlFlowIterator(NULL),
    generateTraining(NULL),
    toSourceString(NULL),
    feedbackAccumulationFunction(NULL),
    feedbackPropogateFunction(NULL),
    generateCppFunction(NULL)
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

int Function::numInputs()
{
    return (int) inputTypes.count();
}

void Function::appendInput(Term* type, std::string const& name)
{
    inputTypes.append(type);
    getInputProperties(inputTypes.count()-1).name = name;
}

void Function::prependInput(Term* type, std::string const& name)
{
    InputProperties props;
    props.name = name;
    inputTypes.prepend(type);
    inputProperties.insert(inputProperties.begin(), props);
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
    copy_field(pureFunction);
    copy_field(hasSideEffects);
    copy_field(variableArgs);
    copy_field(name);
    copy_field(evaluate);
    copy_field(specializeType);
    copy_field(generateTraining);
    copy_field(toSourceString);
    copy_field(startControlFlowIterator);
    copy_field(feedbackAccumulationFunction);
    copy_field(feedbackPropogateFunction);
    copy_field(generateCppFunction);

#undef copy_field
}

void Function::assign(Term* sourceTerm, Term* destTerm)
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
    func.subroutineBranch.remapPointers(map);
}


std::string get_placeholder_name_for_index(int index)
{
    std::stringstream sstream;
    sstream << INPUT_PLACEHOLDER_PREFIX << index;
    return sstream.str();
}

std::string
Function::functionToSourceString(Term* term)
{
    Function &func = as_function(term);

    std::stringstream result;

    result << "def " << func.name << "(";

    bool first = true;
    for (int i=0; i < func.numInputs(); i++) {
        std::string name = func.getInputProperties(i).name;

        if (name == "#state")
            continue;

        if (!first) result << ", ";
        first = false;
        result << func.inputType(i)->name;

        if (func.getInputProperties(i).name != "")
            result << " " << name;
    }

    result << ")";

    if (func.outputType != VOID_TYPE)
        result << " : " << func.outputType->name;

    result << "\n";

    result << get_branch_source(func.subroutineBranch);
    
    result << "end";

    return result.str();
}

Term* create_empty_function(Branch& branch, std::string const& header)
{
    return parser::compile(&branch, parser::function_from_header, header);
}

} // namespace circa
