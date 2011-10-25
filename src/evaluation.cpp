// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "building.h"
#include "build_options.h"
#include "builtins.h"
#include "branch.h"
#include "code_iterators.h"
#include "evaluation.h"
#include "function.h"
#include "introspection.h"
#include "list_shared.h"
#include "locals.h"
#include "parser.h"
#include "refactoring.h"
#include "stateful_code.h"
#include "term.h"
#include "type.h"
#include "types/dict.h"

namespace circa {

Type stackVariableIsn_t;
Type globalVariableIsn_t;

struct StackVariable {
    short relativeFrame;
    short index;
};

EvalContext::EvalContext()
 : interruptSubroutine(false),
   errorOccurred(false),
   numFrames(0),
   stack2(NULL),
   currentTerm(NULL)
{
    register_new_object((CircaObject*) this, &EVAL_CONTEXT_T, true);
}

EvalContext::~EvalContext()
{
    on_object_deleted((CircaObject*) this);
}

void eval_context_list_references(CircaObject* object, GCReferenceList* list, GCColor color)
{
    // todo
}

std::string stackVariable_toString(TaggedValue* value)
{
    short relativeFrame = value->value_data.asint << 16;
    short index = (value->value_data.asint & 0xffff);
    std::stringstream strm;
    strm << "[frame:" << relativeFrame << ", index:" << index << "]";
    return strm.str();
}

void eval_context_setup_type(Type* type)
{
    type->name = "EvalContext";
    type->gcListReferences = eval_context_list_references;

    stackVariableIsn_t.name = "stackVariableIsn";
    stackVariableIsn_t.storageType = STORAGE_TYPE_INT;
    stackVariableIsn_t.toString = stackVariable_toString;
    globalVariableIsn_t.name = "globalVariableIsn";
    globalVariableIsn_t.storageType = STORAGE_TYPE_REF;
}

Frame* get_frame(EvalContext* context, int depth)
{
    ca_assert(depth < context->numFrames);
    return &context->stack2[context->numFrames - 1 - depth];
}
Frame* push_frame(EvalContext* context, Branch* branch)
{
    context->numFrames++;
    context->stack2 = (Frame*) realloc(context->stack2, sizeof(Frame) * context->numFrames);
    Frame* top = &context->stack2[context->numFrames - 1];
    initialize_null(&top->registers);
    initialize_null(&top->state);
    set_list(&top->registers, get_locals_count(branch));
    set_dict(&top->state);
    top->branch = branch;
    return top;
}
void pop_frame(EvalContext* context)
{
    Frame* top = &context->stack2[context->numFrames - 1];
    set_null(&top->registers);
    set_null(&top->state);
    context->numFrames--;
}
Frame* top_frame(EvalContext* context)
{
    return get_frame(context, 0);
}

void write_stack_input_instruction(Branch* callingFrame, Term* input, TaggedValue* isn)
{
    change_type_no_initialize(isn, &stackVariableIsn_t);
    int relativeFrame = get_frame_distance(callingFrame, input);
    
    isn->value_data.asint = 0;
    isn->value_data.asint += relativeFrame << 16;
    isn->value_data.asint += (input->localsIndex % 0xffff);
}

void evaluate_single_term(EvalContext* context, Term* term)
{
    if (term->function == NULL)
        return;

    Function* function = get_function_attrs(term->function);

    if (function == NULL)
        return;

    if (function->evaluate == NULL)
        return;

    context->currentTerm = term;

    // Prepare the argument list
    int inputCount = term->numInputs();
    int argCount = inputCount + 1;
    ListData* argList = allocate_list(argCount);

    for (int i=0; i < inputCount; i++) {
        Term* input = term->input(i);
        if (input == NULL) {
            set_null(list_get_index(argList,i));
        } else if (is_value(input)) {
            set_pointer(list_get_index(argList, i), &globalVariableIsn_t, input);
        } else {
            write_stack_input_instruction(term->owningBranch, input, list_get_index(argList,i));
        }
    }

    // Prepare output
    write_stack_input_instruction(term->owningBranch, term, list_get_index(argList, inputCount));

    #if CIRCA_THROW_ON_ERROR
    try {
    #endif

    function->evaluate(context, argCount, argList->items);

    #if CIRCA_THROW_ON_ERROR
    } catch (std::exception const& e) { return error_occurred(context, term, e.what()); }
    #endif

    // For a test build, we check the type of the output of every single call. This is
    // slow, and it should be unnecessary if the function is written correctly. But it's
    // a good test.
    #ifdef CIRCA_TEST_BUILD
    if (!context->errorOccurred && !is_value(term)) {
        for (int i=0; i < get_output_count(term); i++) {

            Type* outputType = get_output_type(term, i);
            TaggedValue* output = get_output(context, term, i);

            // Special case, if the function's output type is void then we don't care
            // if the output value is null or not.
            if (i == 0 && outputType == &VOID_T)
                continue;

            if (!cast_possible(output, outputType)) {
                std::stringstream msg;
                msg << "Function " << term->function->name << " produced output "
                    << output->toString() << " (in index " << i << ")"
                    << " which doesn't fit output type "
                    << outputType->name;
                internal_error(msg.str());
            }
        }
    }
    #endif

    free_list(argList);
}

void evaluate_branch_internal(EvalContext* context, Branch* branch)
{
    push_frame(context, branch);

    for (int i=0; i < branch->length(); i++) {
        evaluate_single_term(context, branch->get(i));

          if (evaluation_interrupted(context))
              break;
    }

    pop_frame(context);
}

void evaluate_branch_internal(EvalContext* context, Branch* branch, TaggedValue* output)
{
    push_frame(context, branch);

    for (int i=0; i < branch->length(); i++)
        evaluate_single_term(context, branch->get(i));

    if (output != NULL)
        copy(get_local(branch->get(branch->length()-1)), output);

    pop_frame(context);
}

void evaluate_branch_internal_with_state(EvalContext* context, Term* term,
        Branch* branch)
{
    // Store currentScopeState and fetch the container for this branch
    TaggedValue prevScopeState;
    swap(&context->currentScopeState, &prevScopeState);
    fetch_state_container(term, &prevScopeState, &context->currentScopeState);

    evaluate_branch_internal(context, branch);

    // Store container and replace currentScopeState
    save_and_consume_state(term, &prevScopeState, &context->currentScopeState);
    swap(&context->currentScopeState, &prevScopeState);
}

void evaluate_branch_no_preserve_locals(EvalContext* context, Branch* branch)
{
    copy(&context->state, &context->currentScopeState);

    evaluate_branch_internal(context, branch);

    swap(&context->currentScopeState, &context->state);
    set_null(&context->currentScopeState);
}

void evaluate_branch(EvalContext* context, Branch* branch)
{
    evaluate_branch_no_preserve_locals(context, branch);

    // Copy stack back to the original terms. Many tests depend on this functionality.
    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);
        if (is_value(term)) continue;
        TaggedValue* val = get_local(term);
        if (val != NULL)
            copy(val, branch->get(i));
    }
}

void evaluate_branch(Branch* branch)
{
    EvalContext context;
    evaluate_branch(&context, branch);
}

TaggedValue* get_input(EvalContext* context, Term* term, int index)
{
    InputInstruction *instruction = &term->inputIsns.inputs[index];

    switch (instruction->type) {
    case InputInstruction::GLOBAL:
        return (TaggedValue*) term->input(index);
    case InputInstruction::OLD_STYLE_LOCAL:
        return get_local(term->input(index), term->inputInfo(index)->outputIndex);
    case InputInstruction::EMPTY:
        return NULL;
    case InputInstruction::LOCAL: {
        TaggedValue* frame = list_get_index_from_end(&context->stack,
            instruction->data.relativeFrame);
        return list_get_index(frame, instruction->data.index);
    }
    case InputInstruction::LOCAL_CONSUME:
    default:
        internal_error("Not yet implemented");
        return NULL;
    }
}

void consume_input(EvalContext* context, Term* term, int index, TaggedValue* dest)
{
    // Temp, don't actually consume
    copy(get_input(context, term, index), dest);
}

TaggedValue* get_output(EvalContext* context, Term* term, int index)
{
    InputInstruction *instruction = &term->inputIsns.outputs[index];

    switch (instruction->type) {
    case InputInstruction::GLOBAL:
        return (TaggedValue*) term;
    case InputInstruction::OLD_STYLE_LOCAL:
        return get_local(term, index);
    case InputInstruction::EMPTY:
        internal_error("Attempt to access NULL output");
        return NULL;
    case InputInstruction::LOCAL: {
        TaggedValue* frame = list_get_index_from_end(&context->stack,
            instruction->data.relativeFrame);
        return list_get_index(frame, instruction->data.index);
    }
    case InputInstruction::LOCAL_CONSUME:
    default:
        internal_error("Invalid instruction in get_output: LOCAL_CONSUME");
        return NULL;
    }
}

TaggedValue* get_extra_output(EvalContext* context, Term* term, int index)
{
    return get_output(context, term, index + 1);
}

TaggedValue* get_state_input(EvalContext* cxt, Term* term)
{
    if (term->input(0) == NULL) {
        Dict* currentScopeState = get_current_scope_state(cxt);
        ca_assert(currentScopeState != NULL);
        return currentScopeState->insert(term->uniqueName.name.c_str());
    } else {
        return get_input(cxt, term, 0);
    }
}

TaggedValue* get_local(Term* term, int outputIndex)
{
    internal_error("don't use get_local");
    return NULL;
}

TaggedValue* get_local(Term* term)
{
    internal_error("don't use get_local");
    return NULL;
}

TaggedValue* get_local_safe(Term* term, int outputIndex)
{
    internal_error("don't use get_local");
    return NULL;
}

TaggedValue* get_arg(EvalContext* context, TaggedValue* args, int index)
{
    TaggedValue* arg = &args[index];
    if (arg->value_type == &globalVariableIsn_t) {
        return (TaggedValue*) get_pointer(arg);
    } else if (arg->value_type == &stackVariableIsn_t) {
        short relativeFrame = arg->value_data.asint << 16;
        short index = arg->value_data.asint & 0xffff;

        ca_assert(relativeFrame < context->numFrames);
        Frame* frame = get_frame(context, relativeFrame);
        return frame->registers[index];
    } else {
        return &args[index];
    }
}

void error_occurred(EvalContext* context, Term* errorTerm, std::string const& message)
{
    // Save the error as this term's output value.
    TaggedValue* out = get_output(context, errorTerm, 0);
    set_string(out, message);
    out->value_type = &ERROR_T;

    // Check if there is an errored() call listening to this term. If so, then
    // continue execution.
    if (has_an_error_listener(errorTerm))
        return;

    if (DEBUG_TRAP_ERROR_OCCURRED)
        ca_assert(false);

    ca_assert(errorTerm != NULL);

    if (context == NULL)
        throw std::runtime_error(message);

    if (!context->errorOccurred) {
        context->errorOccurred = true;
        context->errorTerm = errorTerm;
    }
}
void print_runtime_error_formatted(EvalContext& context, std::ostream& output)
{
    output << get_short_location(context.errorTerm)
        << " " << context_get_error_message(&context);
}

Dict* get_current_scope_state(EvalContext* cxt)
{
    return Dict::lazyCast(&cxt->currentScopeState);
}

void fetch_state_container(Term* term, TaggedValue* container, TaggedValue* output)
{
    Dict* containerDict = Dict::lazyCast(container);
    copy(containerDict->insert(term->uniqueName.name.c_str()), output);
}

void save_and_consume_state(Term* term, TaggedValue* container, TaggedValue* result)
{
    Dict* containerDict = Dict::lazyCast(container);
    const char* name = term->uniqueName.name.c_str();
    swap(result, containerDict->insert(name));
    set_null(result);
}

bool evaluation_interrupted(EvalContext* context)
{
    return context->errorOccurred || context->interruptSubroutine
        || context->forLoopContext.breakCalled || context->forLoopContext.continueCalled;
}

void evaluate_range(EvalContext* context, Branch* branch, int start, int end)
{
    push_frame(context, branch);

    for (int i=start; i <= end; i++)
        evaluate_single_term(context, branch->get(i));

    // copy locals back to terms
    for (int i=start; i <= end; i++) {
        Term* term = branch->get(i);
        if (is_value(term))
            continue;
        TaggedValue* value = get_local(term);
        if (value == NULL)
            continue;
        copy(value, term);
    }

    pop_frame(context);
}

void evaluate_minimum(EvalContext* context, Term* term, TaggedValue* result)
{
    // Get a list of every term that this term depends on. Also, limit this
    // search to terms inside the current branch.
    
    Branch* branch = term->owningBranch;

    push_frame(context, branch);

    bool *marked = new bool[branch->length()];
    memset(marked, false, sizeof(bool)*branch->length());

    marked[term->index] = true;

    for (int i=term->index; i >= 0; i--) {
        Term* checkTerm = branch->get(i);
        if (checkTerm == NULL)
            continue;

        if (marked[i]) {
            for (int inputIndex=0; inputIndex < checkTerm->numInputs(); inputIndex++) {
                Term* input = checkTerm->input(inputIndex);
                if (input == NULL)
                    continue;
                if (input->owningBranch != branch)
                    continue;
                // don't follow :meta inputs
                if (function_get_input_meta(get_function_attrs(checkTerm->function),
                            inputIndex))
                    continue;
                marked[input->index] = true;
            }
        }
    }

    for (int i=0; i <= term->index; i++) {
        if (marked[i])
            evaluate_single_term(context, branch->get(i));
    }

    // Possibly save output
    if (result != NULL)
        copy(get_local(term), result);

    delete[] marked;

    pop_frame(context);
}

TaggedValue* evaluate(EvalContext* context, Branch* branch, std::string const& input)
{
    int prevHead = branch->length();
    Term* result = parser::compile(branch, parser::statement_list, input);
    evaluate_range(context, branch, prevHead, branch->length() - 1);
    return get_local(result);
}

TaggedValue* evaluate(Branch* branch, Term* function, List* inputs)
{
    EvalContext context;

    TermList inputTerms;
    inputTerms.resize(inputs->length());

    for (int i=0; i < inputs->length(); i++)
        inputTerms.setAt(i, create_value(branch, inputs->get(i)));

    int prevHead = branch->length();
    Term* result = apply(branch, function, inputTerms);
    evaluate_range(&context, branch, prevHead, branch->length() - 1);
    return get_local(result);
}

TaggedValue* evaluate(Term* function, List* inputs)
{
    Branch scratch;
    return evaluate(&scratch, function, inputs);
}

void clear_error(EvalContext* cxt)
{
    cxt->errorOccurred = false;
    cxt->errorTerm = NULL;
}

std::string context_get_error_message(EvalContext* cxt)
{
    ca_assert(cxt != NULL);
    ca_assert(cxt->errorTerm != NULL);
    TaggedValue* output = get_output(cxt, cxt->errorTerm, 0);
    if (output == NULL)
        return "error message unavailable";
    return as_string(output);
}

} // namespace circa
