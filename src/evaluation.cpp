// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "building.h"
#include "build_options.h"
#include "kernel.h"
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
Type stateInputIsn_t;

struct StackVariable {
    short relativeFrame;
    short index;
};

EvalContext::EvalContext()
 : interruptSubroutine(false),
   errorOccurred(false),
   numFrames(0),
   stack(NULL),
   currentTerm(NULL)
{
    register_new_object((CircaObject*) this, &EVAL_CONTEXT_T, true);
}

EvalContext::~EvalContext()
{
    on_object_deleted((CircaObject*) this);
}

std::string stackVariable_toString(TaggedValue* value)
{
    short relativeFrame = value->value_data.asint >> 16;
    short index = (value->value_data.asint & 0xffff);
    std::stringstream strm;
    strm << "[frame:" << relativeFrame << ", index:" << index << "]";
    return strm.str();
}

void eval_context_list_references(CircaObject* object, GCReferenceList* list, GCColor color)
{
    // todo
}

void eval_context_print_multiline(std::ostream& out, EvalContext* context)
{
    out << "[EvalContext " << context << "]" << std::endl;
    for (int frameIndex = 0; frameIndex < context->numFrames; frameIndex++) {
        Frame* frame = get_frame(context, context->numFrames - 1 - frameIndex);
        Branch* branch = frame->branch;
        out << " [Frame " << frameIndex << ", branch = " << branch << "]" << std::endl;

        if (branch == NULL)
            continue;

        for (int i=0; i < branch->length(); i++) {
            Term* term = branch->get(i);

            // indent
            for (int x = 0; x < frameIndex+1; x++)
                out << " ";

            print_term(out, term);

            // current value
            if (!is_value(term)) {
                TaggedValue* value = frame->registers[term->index];
                if (value == NULL)
                    out << " [<register OOB>]";
                else
                    out << "  [" << value->toString() << "]";
            }
            out << std::endl;
        }
    }
}

void eval_context_setup_type(Type* type)
{
    type->name = "EvalContext";
    type->gcListReferences = eval_context_list_references;

    stackVariableIsn_t.name = "StackVariableIsn";
    stackVariableIsn_t.storageType = STORAGE_TYPE_INT;
    stackVariableIsn_t.toString = stackVariable_toString;
    globalVariableIsn_t.name = "GlobalVariableIsn";
    globalVariableIsn_t.storageType = STORAGE_TYPE_REF;
    stateInputIsn_t.name = "StateInputIsn";
}

Frame* get_frame(EvalContext* context, int depth)
{
    ca_assert(depth >= 0);
    ca_assert(depth < context->numFrames);
    return &context->stack[context->numFrames - 1 - depth];
}
Frame* get_frame_from_bottom(EvalContext* context, int index)
{
    ca_assert(index >= 0);
    ca_assert(index < context->numFrames);
    return &context->stack[index];
}
Frame* push_frame(EvalContext* context, Branch* branch, List* registers)
{
    context->numFrames++;
    context->stack = (Frame*) realloc(context->stack, sizeof(Frame) * context->numFrames);
    Frame* top = &context->stack[context->numFrames - 1];
    initialize_null(&top->registers);
    swap(registers, &top->registers);
    //initialize_null(&top->state);
    //set_dict(&top->state);
    top->branch = branch;
    return top;
}
Frame* push_frame(EvalContext* context, Branch* branch)
{
    List registers;
    registers.resize(get_locals_count(branch));
    return push_frame(context, branch, &registers);
}
void pop_frame(EvalContext* context)
{
    Frame* top = &context->stack[context->numFrames - 1];

    // Check to make sure we aren't losing a stored runtime error.
    if (context->errorOccurred && context->errorTerm->owningBranch == top->branch)
        internal_error("pop_frame called to pop an errored frame");

    set_null(&top->registers);
    //set_null(&top->state);
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

    // Special case: if a term in a #joining branch is trying to reach a neighboring
    // branch, then that's okay.
    if (relativeFrame == -1 && callingFrame->owningTerm->name == "#joining")
        relativeFrame = 0;
    
    isn->value_data.asint = 0;
    isn->value_data.asint += relativeFrame << 16;
    isn->value_data.asint += (input->index % 0xffff);
}

void write_state_input_instruction(TaggedValue* isn)
{
    change_type_no_initialize(isn, &stateInputIsn_t);
}

void write_input_instruction(Term* caller, Term* input, TaggedValue* isn)
{
    if (input == NULL) {
        set_null(isn);
    } else if (is_value(input)) {
        set_pointer(isn, &globalVariableIsn_t, input);
    } else {
        write_stack_input_instruction(caller->owningBranch, input, isn);
    }
}

ListData* write_input_instruction_list(Term* caller, ListData* list)
{
    Function* func = as_function(caller->function);

    // Walk through each of the function's declared inputs, and write appropriate
    // instructions.
    int callerIndex = 0;
    int writeIndex = 0;
    int declaredCount = function_num_inputs(func);

    for (int declaredIndex=0; declaredIndex < declaredCount; declaredIndex++) {


        if (function_is_state_input(func, declaredIndex)) {
            list = list_resize(list, writeIndex + 1);
            TaggedValue* isn = list_get_index(list, writeIndex);
            write_state_input_instruction(isn);

        } else if (function_is_multiple_input(func, declaredIndex)) {

            // Write the remainder of caller's arguments.
            while (callerIndex < caller->numInputs()) {

                list = list_resize(list, writeIndex + 1);
                TaggedValue* isn = list_get_index(list, writeIndex);
                write_input_instruction(caller, caller->input(callerIndex), isn);
                callerIndex++;
                writeIndex++;
            }
            break;

        } else {
            // Write a normal input.
            list = list_resize(list, writeIndex + 1);
            TaggedValue* isn = list_get_index(list, writeIndex);
            write_input_instruction(caller, caller->input(callerIndex), isn);
            callerIndex++;
        }

        writeIndex++;
    }
    return list;
}

void evaluate_single_term(EvalContext* context, Term* term)
{
    if (term->function == NULL)
        return;

    Function* function = as_function(term->function);

    if (function == NULL || function->evaluate == NULL)
        return;

    context->currentTerm = term;

    // Prepare input list
    ListData* inputList = write_input_instruction_list(term, NULL);

    // Prepare output list
    ListData* outputList = allocate_list(1);
    write_stack_input_instruction(term->owningBranch, term, list_get_index(outputList, 0));

    #if CIRCA_THROW_ON_ERROR
    try {
    #endif

    function->evaluate(context, inputList, outputList);

    #if CIRCA_THROW_ON_ERROR
    } catch (std::exception const& e) { return error_occurred(context, term, e.what()); }
    #endif

    // For a test build, we check the type of the output of every single call. This is
    // slow, and it should be unnecessary if the function is written correctly. But it's
    // a good test.
    #ifdef CIRCA_TEST_BUILD
    if (!context->errorOccurred && !is_value(term)) {
        Type* outputType = get_output_type(term);
        TaggedValue* output = get_arg(context, outputList, 0);

        if (outputType != &VOID_T && !cast_possible(output, outputType)) {
            std::stringstream msg;
            msg << "Function " << term->function->name << " produced output "
                << output->toString() << " which doesn't fit output type "
                << outputType->name;
            internal_error(msg.str());
        }
    }
    #endif

    free_list(inputList);
    free_list(outputList);
}

void evaluate_branch_internal(EvalContext* context, Branch* branch)
{
    push_frame(context, branch);

    for (int i=0; i < branch->length(); i++) {
        evaluate_single_term(context, branch->get(i));

          if (evaluation_interrupted(context))
              break;
    }

    if (!context->errorOccurred)
        pop_frame(context);
}

void evaluate_branch_internal(EvalContext* context, Branch* branch, TaggedValue* output)
{
    push_frame(context, branch);

    for (int i=0; i < branch->length(); i++)
        evaluate_single_term(context, branch->get(i));

    if (output != NULL) {
        Term* outputTerm = branch->getFromEnd(0);
        copy(top_frame(context)->registers[outputTerm->index], output);
    }

    if (!context->errorOccurred)
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

void copy_locals_back_to_terms(Frame* frame, Branch* branch)
{
    // Copy stack back to the original terms. Many tests depend on this functionality.
    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);
        if (is_value(term)) continue;
        TaggedValue* val = frame->registers[term->index];
        if (val != NULL)
            copy(val, branch->get(i));
    }
}

void evaluate_branch(EvalContext* context, Branch* branch)
{
    evaluate_branch_no_preserve_locals(context, branch);
}

void evaluate_save_locals(EvalContext* context, Branch* branch)
{
    push_frame(context, branch);

    for (int i=0; i < branch->length(); i++) {
        evaluate_single_term(context, branch->get(i));

          if (evaluation_interrupted(context))
              break;
    }

    copy_locals_back_to_terms(top_frame(context), branch);

    if (!context->errorOccurred)
        pop_frame(context);
}

void evaluate_branch(Branch* branch)
{
    EvalContext context;
    evaluate_save_locals(&context, branch);
}

TaggedValue* get_input(EvalContext* context, Term* term, int index)
{
    internal_error("Don't call get_input");
    return NULL;
}

void consume_input(EvalContext* context, Term* term, int index, TaggedValue* dest)
{
    // Temp, don't actually consume
    copy(get_input(context, term, index), dest);
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

TaggedValue* get_arg(EvalContext* context, TaggedValue* arg)
{
    if (arg->value_type == &globalVariableIsn_t) {
        return (TaggedValue*) get_pointer(arg);
    } else if (arg->value_type == &stackVariableIsn_t) {
        short relativeFrame = arg->value_data.asint >> 16;
        short index = arg->value_data.asint & 0xffff;

        ca_assert(relativeFrame < context->numFrames);
        ca_assert(relativeFrame >= 0);
        Frame* frame = get_frame(context, relativeFrame);
        return frame->registers[index];
    } else {
        return arg;
    }
}

TaggedValue* get_arg(EvalContext* context, ListData* args, int index)
{
    return get_arg(context, list_get_index(args, index));
}
void consume_arg(EvalContext* context, ListData* args, int index, TaggedValue* dest)
{
    // TODO: Make this swap() values when possible
    copy(get_arg(context, list_get_index(args, index)), dest);
}
TaggedValue* get_output(EvalContext* context, ListData* args)
{
    return get_arg(context, args, list_size(args) - 1);
}
TaggedValue* get_extra_output(EvalContext* context, Term* term, int index)
{
    internal_error("get_extra_output no worky");
    return NULL;
}

void error_occurred(EvalContext* context, Term* term, TaggedValue* output, const char* message)
{
    // Save the error as this term's output value.
    set_string(output, message);
    output->value_type = &ERROR_T;

    // Check if there is an errored() call listening to this term. If so, then
    // continue execution.
    if (has_an_error_listener(term))
        return;

    if (DEBUG_TRAP_ERROR_OCCURRED)
        ca_assert(false);

    if (context == NULL)
        throw std::runtime_error(message);

    if (!context->errorOccurred) {
        context->errorOccurred = true;
        context->errorTerm = term;
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

// Old style state manipulation:
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

#if 0
// New style state manipulation:
void fetch_stack_local_state(EvalContext* context, const char* name)
{
    ca_assert(context->numFrames > 1);
    Frame* frame = top_frame(context);
    Frame* parentFrame = get_frame(context, 1);

    copy(parentFrame->state.get(name), &frame->state);
    if (!is_dict(&frame->state))
        set_dict(&frame->state);
}
void store_stack_local_state(EvalContext* context, const char* name)
{
    ca_assert(context->numFrames > 1);
    Frame* frame = top_frame(context);
    Frame* parentFrame = get_frame(context, 1);

    copy(&frame->state, parentFrame->state.insert(name));
}
#endif

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

    if (context->errorOccurred)
        return;

    copy_locals_back_to_terms(top_frame(context), branch);
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
                if (function_get_input_meta(as_function(checkTerm->function),
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
        copy(top_frame(context)->registers[term->index], result);

    delete[] marked;

    pop_frame(context);
}

TaggedValue* evaluate(EvalContext* context, Branch* branch, std::string const& input)
{
    int prevHead = branch->length();
    Term* result = parser::compile(branch, parser::statement_list, input);
    evaluate_range(context, branch, prevHead, branch->length() - 1);
    return result;
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
    return result;
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
    ca_assert(cxt->numFrames > 0);

    Frame* frame = top_frame(cxt);

    if (cxt->errorTerm->owningBranch != frame->branch)
        internal_error("called context_get_error_message, but the errored frame is gone");

    return as_string(frame->registers[cxt->errorTerm->index]);
}

} // namespace circa
