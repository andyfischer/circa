// Copyright 2008 Andrew Fischer

#include "branch.h"
#include "builtins.h"
#include "function.h"
#include "importing.h"
#include "list.h"
#include "parser.h"
#include "pointer_iterator.h"
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
    evaluate(NULL),
    startControlFlowIterator(NULL),
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

void Function::duplicate(Term* sourceTerm, Term* destTerm)
{
    assert(destTerm->value == NULL);
    assert(is_function(sourceTerm));
    assert(is_function(destTerm));
    destTerm->value = new Function();

    Function &source = as_function(sourceTerm);
    Function &dest = as_function(destTerm);

    dest.inputTypes =      source.inputTypes;
    dest.inputProperties = source.inputProperties;
    dest.outputType =      source.outputType;
    dest.stateType =       source.stateType;
    dest.pureFunction =    source.pureFunction;
    dest.hasSideEffects =  source.hasSideEffects;
    dest.variableArgs =    source.variableArgs;
    dest.name =            source.name;
    dest.evaluate =        source.evaluate;
    dest.startControlFlowIterator =        source.startControlFlowIterator;
    dest.feedbackAccumulationFunction =    source.feedbackAccumulationFunction;
    dest.feedbackPropogateFunction =       source.feedbackPropogateFunction;
    dest.generateCppFunction =             source.generateCppFunction;
    dest.printCircaSourceFunction =        source.printCircaSourceFunction;

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
    if (branch.containsName(OUTPUT_PLACEHOLDER_NAME)) {
        Term* outputPlaceholder = branch[OUTPUT_PLACEHOLDER_NAME];
        assert(outputPlaceholder != NULL);
        recycle_value(outputPlaceholder, caller);
    }
}

PointerIterator*
Function::subroutine_call_start_control_flow_iterator(Term* caller)
{
    Branch &branch = as_branch(caller->state);
    return start_branch_control_flow_iterator(&branch);
}

Branch& get_subroutine_branch(Term* term)
{
    return as_function(term).subroutineBranch;
}

class FunctionPointerIterator : public PointerIterator
{
private:
    enum Step { INPUT_TYPES, OUTPUT_TYPE, STATE_TYPE, SUBROUTINE_BRANCH };
    Function* _function;
    Step _step;
    int _inputIndex;
    PointerIterator *_subroutineBranchIterator;
public:
    FunctionPointerIterator(Function* function)
      : _function(function),
        _step(INPUT_TYPES),
        _inputIndex(0),
        _subroutineBranchIterator(NULL)
    {
        advanceIfStateIsInvalid();
    }

    virtual Term* current()
    {
        assert(!finished());

        switch (_step) {
        case INPUT_TYPES:
            return _function->inputTypes[_inputIndex];
        case OUTPUT_TYPE:
            return _function->outputType;
        case STATE_TYPE:
            return _function->stateType;
        case SUBROUTINE_BRANCH:
            return _subroutineBranchIterator->current();
        }

        throw std::runtime_error("internal error");
    }

    virtual void advance()
    {
        assert(!finished());
        internalAdvance();

        while(!finished() && current() == NULL)
            internalAdvance();
    }
    
    void internalAdvance()
    {
        switch (_step) {
        case INPUT_TYPES:
            _inputIndex++;
            break;
        case OUTPUT_TYPE:
            _step = STATE_TYPE;
            break;
        case STATE_TYPE:
            _step = SUBROUTINE_BRANCH;
            _subroutineBranchIterator = start_branch_pointer_iterator(&_function->subroutineBranch);
            if (_subroutineBranchIterator == NULL)
                _function = NULL;
            break;
        case SUBROUTINE_BRANCH:
            _subroutineBranchIterator->advance();
            break;
        }

        advanceIfStateIsInvalid();
    }
    virtual bool finished()
    {
        return _function == NULL;
    }

private:
    void advanceIfStateIsInvalid()
    {
        switch(_step) {
        case INPUT_TYPES:
            if (_inputIndex >= (int) _function->inputTypes.count()) {
                _step = OUTPUT_TYPE;
            }
            return;
        case OUTPUT_TYPE:
            return;
        case STATE_TYPE:
            return;
        case SUBROUTINE_BRANCH:
            if (_subroutineBranchIterator->finished()) {
                delete _subroutineBranchIterator;
                _subroutineBranchIterator = NULL;
                _function = NULL;
            }
        }
    }
};

PointerIterator* start_function_pointer_iterator(Function* function)
{
    return new FunctionPointerIterator(function);
}

PointerIterator* start_control_flow_iterator(Term* term)
{
    Function &func = as_function(term->function);

    if (func.startControlFlowIterator == NULL)
        return NULL;

    return func.startControlFlowIterator(term);
}

} // namespace circa
