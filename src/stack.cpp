// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "bytecode.h"
#include "closures.h"
#include "hashtable.h"
#include "if_block.h"
#include "inspection.h"
#include "interpreter.h"
#include "kernel.h"
#include "list.h"
#include "migration.h"
#include "names.h"
#include "native_patch.h"
#include "stack.h"
#include "string_type.h"
#include "symbols.h"
#include "tagged_value.h"
#include "type.h"
#include "world.h"

namespace circa {

Stack::Stack()
 : errorOccurred(false),
   world(NULL)
{
    world = global_world();
    id = global_world()->nextStackID++;

    refCount = 1;
    isRefcounted = false;

    step = sym_StackReady;

    frames = NULL;
    frameCount = 0;
    frameCapacity = 0;

    registers = NULL;
    registerCount = 0;
    registerCapacity = 0;

    nextStack = NULL;
    prevStack = NULL;

    caller = NULL;
    
    set_hashtable(&demandValues);
    set_hashtable(&env);
    set_hashtable(&state);
    set_hashtable(&observations);
    rand_init(&randState, 0);

    set_hashtable(&attrs);

    // Currently: each Stack owns their own Program. Future: We'll
    // share Programs among stacks.
    program = alloc_program(global_world());
}

Stack::~Stack()
{
    // Clear error, so that pop_frame doesn't complain about losing an errored frame.
    this->errorOccurred = false;

    stack_on_program_change(this);
    stack_reset(this);

    free(frames);
    free(registers);

    free_program(program);

    if (world != NULL) {
        if (world->firstStack == this)
            world->firstStack = world->firstStack->nextStack;
        if (world->lastStack == this)
            world->lastStack = world->lastStack->prevStack;
        if (nextStack != NULL)
            nextStack->prevStack = prevStack;
        if (prevStack != NULL)
            prevStack->nextStack = nextStack;
    }
}

void
Stack::dump()
{
    circa::dump(this);
}
void
Stack::dump_compiled()
{
    compiled_dump(program);
}

Stack* create_stack(World* world)
{
    Stack* stack = new Stack();

    stack->world = world;

    stack->isRefcounted = true;
    stack->refCount = 1;
    
    // Add to list of stacks in World.
    if (world != NULL) {
        if (world->firstStack == NULL)
            world->firstStack = stack;
        if (world->lastStack != NULL)
            world->lastStack->nextStack = stack;
        stack->prevStack = world->lastStack;
        stack->nextStack = NULL;
        world->lastStack = stack;
    }

    return stack;
}

void free_stack(Stack* stack)
{
    delete stack;
}

void stack_incref(Stack* stack)
{
    if (stack->isRefcounted) {
        ca_assert(stack->refCount > 0);
        stack->refCount++;
    }
}

void stack_decref(Stack* stack)
{
    if (stack->isRefcounted) {
        ca_assert(stack->refCount > 0);
        stack->refCount--;
        if (stack->refCount == 0)
            free_stack(stack);
    }
}

void stack_reserve_frame_capacity(Stack* stack, int desiredCapacity)
{
    if (stack->frameCapacity < desiredCapacity) {
        stack->frames = (Frame*) realloc(stack->frames, sizeof(Frame) * desiredCapacity);
        stack->frameCapacity = desiredCapacity;
    }
}

void stack_reserve_register_capacity(Stack* stack, int desiredSize)
{
    if (stack->registerCapacity < desiredSize) {
        int prevSize = stack->registerCapacity;
        stack->registers = (Value*) realloc(stack->registers, sizeof(Value) * desiredSize);
        for (int i=prevSize; i < desiredSize; i++)
            initialize_null(&stack->registers[i]);
        stack->registerCapacity = desiredSize;
    }
}

Frame* stack_push_blank_frame(Stack* stack, int registerCount)
{
    stat_increment(StackPushFrame);
    stack->frameCount++;
    stack_reserve_frame_capacity(stack, stack->frameCount);

    Frame* frame = top_frame(stack);

    // Prepare frame.
    frame->stack = stack;
    frame->termIndex = 0;
    frame->block = NULL;
    frame->blockIndex = -1;
    frame->pc = 0;
    initialize_null(&frame->bindings);
    initialize_null(&frame->env);
    initialize_null(&frame->incomingState);
    initialize_null(&frame->outgoingState);

    frame->firstRegisterIndex = stack->registerCount;
    frame->registerCount = registerCount;
    stack->registerCount += registerCount;
    stack_reserve_register_capacity(stack, stack->registerCount);

    return frame;
}

void stack_resize_top_frame(Stack* stack, int registerCount)
{
    Frame* top = top_frame(stack);
    top->registerCount = registerCount;
    stack->registerCount = top->firstRegisterIndex + registerCount;
    stack_reserve_register_capacity(stack, stack->registerCount);
}

void pop_frame(Stack* stack)
{
    Frame* frame = top_frame(stack);

    for (int i=0; i < frame->registerCount; i++)
        set_null(frame_register(frame, i));

    stack->registerCount -= frame->registerCount;

    set_null(&frame->bindings);
    set_null(&frame->env);
    set_null(&frame->incomingState);
    set_null(&frame->outgoingState);

    stack->frameCount--;
}

Frame* stack_resize_frame(Stack* stack, Frame* frame, int newRegisterCount)
{
    if (frame->registerCount == newRegisterCount)
        return frame;

    int oldRegisterCount = frame->registerCount;
    int countDelta = newRegisterCount - oldRegisterCount;
    int firstRegisterToMove = frame->firstRegisterIndex + oldRegisterCount;
    int lastRegisterToMove = stack->registerCount;

    if (countDelta > 0) {
        stack->registerCount += countDelta;
        stack_reserve_register_capacity(stack, stack->registerCount);
    }

    frame->registerCount = newRegisterCount;

    // Move existing values

    if (countDelta < 0)
        for (int i = firstRegisterToMove; i < lastRegisterToMove; i++)
            move(stack_register(stack, i), stack_register(stack, i + countDelta));
    else
        for (int i = lastRegisterToMove-1; i >= firstRegisterToMove; i--)
            move(stack_register(stack, i), stack_register(stack, i + countDelta));

    // Update firstRegisterIndex values.
    for (Frame* followingFrame = next_frame(frame);
        followingFrame != NULL; followingFrame = next_frame(followingFrame)) {

        followingFrame->firstRegisterIndex += countDelta;
    }

    if (countDelta < 0)
        stack->registerCount += countDelta;

    return frame;
}

int stack_frame_count(Stack* stack)
{
    return stack->frameCount;
}

Frame* top_frame_parent(Stack* stack)
{
    Frame* top = top_frame(stack);
    if (top == NULL)
        return NULL;
    return prev_frame(top);
}

Block* stack_top_block(Stack* stack)
{
    Frame* frame = top_frame(stack);
    if (frame == NULL)
        return NULL;
    return frame_block(frame);
}

caValue* stack_state(Stack* stack)
{
    return &stack->state;
}

bool stack_errored(Stack* stack)
{
    return stack->errorOccurred;
}

caValue* stack_env_insert(Stack* stack, caValue* name)
{
    return hashtable_insert(&stack->env, name);
}

caValue* stack_env_get(Stack* stack, caValue* name)
{
    return hashtable_get(&stack->env, name);
}

void stack_extract_current_path(Stack* stack, caValue* path, Frame* untilFrame)
{
    set_list(path);

    for (Frame* frame = first_frame(stack); frame != NULL; frame = next_frame(frame)) {
        if (frame == untilFrame)
            break;

        Block* block = frame_block(frame);
        if (is_case_block(block)) {
            set_int(list_append(path), case_block_get_index(block));
        } else if (is_for_loop(block)) {
            Term* index = for_loop_find_index(block);
            set_value(list_append(path), frame_register(frame, index));
        }

        set_term_ref(list_append(path), frame_current_term(frame));
    }
}

Stack* stack_duplicate(Stack* stack)
{
    Stack* dupe = create_stack(stack->world);
    
    set_value(&dupe->attrs, &stack->attrs);
    set_value(&dupe->demandValues, &stack->demandValues);
    set_value(&dupe->env, &stack->env);
    set_value(&dupe->observations, &stack->observations);
    dupe->step = stack->step;
    dupe->errorOccurred = stack->errorOccurred;
    dupe->randState = stack->randState;

    stack_reserve_frame_capacity(dupe, stack->frameCount);
    stack_reserve_register_capacity(dupe, stack->registerCount);

    Frame* sourceFrame = first_frame(stack);

    while (sourceFrame != NULL) {
        Frame* dupeFrame = stack_push_blank_frame(dupe, frame_register_count(sourceFrame));

        for (int i=0; i < frame_register_count(sourceFrame); i++)
            set_value(frame_register(dupeFrame, i), frame_register(sourceFrame, i));

        dupeFrame->parentIndex = sourceFrame->parentIndex;
        dupeFrame->block = sourceFrame->block;
        dupeFrame->termIndex = sourceFrame->termIndex;
        set_value(&dupeFrame->bindings, &sourceFrame->bindings);
        set_value(&dupeFrame->env, &sourceFrame->env);
        set_value(&dupeFrame->incomingState, &sourceFrame->incomingState);
        set_value(&dupeFrame->outgoingState, &sourceFrame->outgoingState);

        sourceFrame = next_frame(sourceFrame);
    }

    return dupe;
}

caValue* stack_active_value_for_block_index(Frame* frame, int blockIndex, int termIndex)
{
    Stack* stack = frame->stack;

    while (true) {
        if (frame->blockIndex == blockIndex) {
            Term* term = frame_term(frame, termIndex);
            if (is_value(term))
                return term_value(term);
            return frame_register(frame, term);
        }

        frame = prev_frame(frame);

        if (frame == NULL)
            break;
    }

    // Check demandValues.
    Block* block = program_block(stack->program, blockIndex);
    Term* term = block->get(termIndex);

    caValue* demandValue = stack_demand_value_get(stack, term);
    if (demandValue != NULL)
        return demandValue;

    return NULL;
}

caValue* stack_active_value_for_term(Frame* frame, Term* term)
{
    int blockIndex = program_find_block_index(frame->stack->program, term->owningBlock);
    if (blockIndex == -1)
        return NULL;
    return stack_active_value_for_block_index(frame, blockIndex, term->index);
}

Frame* first_frame(Stack* stack)
{
    if (stack->frameCount == 0)
        return NULL;
    return (Frame*) stack->frames;
}

Frame* next_frame(Frame* frame)
{
    Stack* stack = frame->stack;
    int frameIndex = (frame - stack->frames) + 1;
    if (frameIndex >= frame->stack->frameCount)
        return NULL;

    return &stack->frames[frameIndex];
}

Frame* next_frame_n(Frame* frame, int distance)
{
    for (; distance > 0; distance--) {
        if (frame == NULL)
            return NULL;

        frame = next_frame(frame);
    }
    return frame;
}

Frame* prev_frame(Frame* frame)
{
    Stack* stack = frame->stack;
    int frameIndex = (frame - stack->frames) - 1;
    if (frameIndex < 0)
        return NULL;

    return &stack->frames[frameIndex];
}

Frame* prev_frame_n(Frame* frame, int distance)
{
    for (; distance > 0; distance--) {
        if (frame == NULL)
            return NULL;

        frame = prev_frame(frame);
    }
    return frame;
}

Frame* top_frame(Stack* stack)
{
    if (stack->frameCount == 0)
        return NULL;
    return &stack->frames[stack->frameCount - 1];
}

CompiledBlock* frame_compiled_block(Frame* frame)
{
    return compiled_block(frame->stack->program, frame->blockIndex);
}

caValue* stack_register(Stack* stack, int index)
{
    ca_assert(index < stack->registerCount);
    return &stack->registers[index];
}

caValue* stack_register_rel(Stack* stack, int relativeIndex)
{
    int index = top_frame(stack)->firstRegisterIndex + relativeIndex;
    ca_assert(index < stack->registerCount);
    return &stack->registers[index];
}

caValue* frame_register(Frame* frame, int index)
{
    ca_assert(index >= 0 && index < frame->registerCount);
    return stack_register(frame->stack, frame->firstRegisterIndex + index);
}

caValue* frame_register(Frame* frame, Term* term)
{
    return frame_register(frame, term->index);
}

caValue* frame_register_from_end(Frame* frame, int index)
{
    return frame_register(frame, frame->registerCount - 1 - index);
}

int frame_register_count(Frame* frame)
{
    return frame->registerCount;
}

void frame_registers_to_list(Frame* frame, caValue* list)
{
    set_list(list, frame_register_count(frame));
    for (int i=0; i < frame_register_count(frame); i++)
        copy(frame_register(frame, i), list_get(list, i));
}

int frame_find_index(Frame* frame)
{
    int index = 0;
    frame = prev_frame(frame);
    while (frame != NULL) {
        index++;
        frame = prev_frame(frame);
    }
    return index;
}

Term* frame_caller(Frame* frame)
{
    return frame_term(prev_frame(frame), frame->parentIndex);
}

Term* frame_current_term(Frame* frame)
{
    return frame_block(frame)->get(frame->termIndex);
}

Term* frame_term(Frame* frame, int index)
{
    return frame_block(frame)->get(index);
}

void stack_bytecode_start_run(Stack* stack)
{
    // Extract the current hackset
    Value hackset;
    stack_derive_hackset(stack, &hackset);

    if (strict_equals(&stack->derivedHackset, &hackset))
        return;

    // Hackset has changed.
    move(&hackset, &stack->derivedHackset);

    stack_on_program_change(stack);

    // Future: Changing hackset will probably cause us to create a different Program, rather than
    // modify the existing one.
    program_set_hackset(stack->program, &stack->derivedHackset);
}

void stack_on_program_change(Stack* stack)
{
    compiled_erase(stack->program);
    for (Frame* frame = first_frame(stack); frame != NULL; frame = next_frame(frame)) {
        frame->pc = 0;
        frame->blockIndex = -1;
    }
    stack->bytecode = NULL;
    stack->pc = 0;
}

void stack_derive_hackset(Stack* stack, Value* hackset)
{
    set_list(hackset);

    caValue* hacks = hashtable_get_symbol_key(&stack->env, sym__hacks);

    if (hacks == NULL)
        return;
    ca_assert(is_list(hacks));

    copy(hacks, hackset);
}

caValue* stack_demand_value_insert(Stack* stack, Term* term)
{
    Value key;
    set_term_ref(&key, term);
    return hashtable_insert(&stack->demandValues, &key);
}

caValue* stack_demand_value_get(Stack* stack, Term* term)
{
    Value key;
    set_term_ref(&key, term);
    return hashtable_get(&stack->demandValues, &key);
}

void stack_save_watch_observation(Stack* stack, Value* path, Value* value)
{
    Value* watches = hashtable_insert_symbol_key(&stack->observations, sym_Watch);
    if (!is_hashtable(watches))
        set_hashtable(watches);
    copy(value, hashtable_insert(watches, path));
}

Value* stack_get_watch_observation(Stack* stack, Value* path)
{
    Value* watches = hashtable_get_symbol_key(&stack->observations, sym_Watch);
    if (watches == NULL)
        return NULL;
    return hashtable_get(watches, path);
}

void stack_on_migration(Stack* stack)
{
    set_hashtable(&stack->demandValues);
    stack_on_program_change(stack);
}

Stack* frame_ref_get_stack(caValue* value)
{
    return as_stack(list_get(value, 0));
}

int frame_ref_get_index(caValue* value)
{
    return as_int(list_get(value, 1));
}

Frame* as_frame_ref(caValue* value)
{
    ca_assert(value->value_type == TYPES.frame);

    Stack* stack = frame_ref_get_stack(value);
    int index = frame_ref_get_index(value);
    if (index >= stack->frameCount)
        return NULL;

    return next_frame_n(first_frame(stack), index);
}

bool is_frame_ref(caValue* value)
{
    return value->value_type == TYPES.frame;
}

void set_frame_ref(caValue* value, Frame* frame)
{
    set_list(value, 2);
    set_stack(list_get(value, 0), frame->stack);
    set_int(list_get(value, 1), frame_find_index(frame));
    cast(value, TYPES.frame);
}

void frame_copy(Frame* left, Frame* right)
{
    right->parentIndex = left->parentIndex;
    copy(&left->bindings, &right->bindings);
    copy(&left->env, &right->env);
    copy(&left->incomingState, &right->incomingState);
    copy(&left->outgoingState, &right->outgoingState);
    right->block = left->block;
    right->blockIndex = left->blockIndex;
    right->termIndex = left->termIndex;
    right->pc = left->pc;
}

void stack_value_copy(Type*, caValue* source, caValue* dest)
{
    Stack* stack = (Stack*) source->value_data.ptr;
    stack_incref(stack);
    make_no_initialize(source->value_type, dest);
    dest->value_data.ptr = stack;
}

void stack_value_release(caValue* value)
{
    Stack* stack = (Stack*) value->value_data.ptr;
    stack_decref(stack);
}

void stack_setup_type(Type* type)
{
    type->initialize = NULL;
    type->release = stack_value_release;
    type->copy = stack_value_copy;
}

void Stack__block(caStack* stack)
{
    Stack* actor = as_stack(circa_input(stack, 0));
    set_block(circa_output(stack, 0), frame_block(top_frame(actor)));
}

void Stack__dump(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    Value str;
    stack_to_string(self, &str, false);
    write_log(as_cstring(&str));
}

void Stack__dump_compiled(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    Value str;
    compiled_to_string(self->program, &str);
    write_log(as_cstring(&str));
}

void Stack__dump_with_bytecode(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    Value str;
    stack_to_string(self, &str, true);
    write_log(as_cstring(&str));
}

void Stack__extract_state(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    copy(&self->state, circa_output(stack, 0));
}

void Stack__eval_on_demand(caStack* stack)
{
#if 0
    Stack* self = as_stack(circa_input(stack, 0));
    Term* term = as_term_ref(circa_input(stack, 1));

    // Make sure currentHacksetBytecode is initialzed
    stack_bytecode_start_run(self);

    vm_evaluate_module_on_demand(self, term, true);
    vm_run(self);
    caValue* result = stack_active_value_for_term(top_frame(self), term);
    copy(result, circa_output(stack, 0));
#endif
}

void Stack__find_active_value(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    Term* term = as_term_ref(circa_input(stack, 1));
    caValue* value = stack_active_value_for_term(top_frame(self), term);

    if (value == NULL)
        set_null(circa_output(stack, 0));
    else
        set_value(circa_output(stack, 0), value);
}

void Stack__find_active_frame_for_term(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    Term* term = as_term_ref(circa_input(stack, 1));

    if (term == NULL)
        return raise_error_msg(stack, "Term is null");

    Frame* frame = top_frame(self);

    while (true) {
        if (frame_block(frame) == term->owningBlock) {
            set_frame_ref(circa_output(stack, 0), frame);
            return;
        }

        frame = prev_frame(frame);

        if (frame == NULL)
            break;
    }

    set_null(circa_output(stack, 0));
}

void Stack__id(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    set_int(circa_output(stack, 0), self->id);
}

void Stack__init(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    caValue* closure = circa_input(stack, 1);
    stack_init_with_closure(self, closure);
}

void Stack__env(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    Value* name = circa_input(stack, 1);

    caValue* value = hashtable_get(&self->env, name);
    if (value == NULL)
        set_null(circa_output(stack, 0));
    else
        copy(value, circa_output(stack, 0));
}

void Stack__env_map(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    copy(&self->env, circa_output(stack, 0));
}

void Stack__set_env(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    caValue* name = circa_input(stack, 1);
    caValue* val = circa_input(stack, 2);

    copy(val, stack_env_insert(self, name));
}

void Stack__set_env_map(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    caValue* map = circa_input(stack, 1);
    copy(map, &self->env);
}

void Stack__get_state(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    copy(&self->state, circa_output(stack, 0));
}

void Stack__get_watch_result(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    Value* path = circa_input(stack, 1);

    Value* observation = stack_get_watch_observation(self, path);

    if (observation == NULL)
        set_null(circa_output(stack, 0));
    else
        set_value(circa_output(stack, 0), observation);
}

void Stack__call(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));

    if (self == NULL)
        return raise_error_msg(self, "Stack is null");

    if (self->caller != NULL)
        return raise_error_msg(self, "Stack already has a call in progress");

    self->caller = stack;

    stack_restart(self);

    // Populate inputs.
    caValue* inputs = circa_input(stack, 1);
    for (int i=0; i < list_length(inputs); i++)
        copy(list_get(inputs, i), circa_input(self, i));

    vm_run(self);

    self->caller = NULL;

    caValue* output = circa_output(self, 0);
    if (output != NULL)
        copy(output, circa_output(stack, 0));
    else
        set_null(circa_output(stack, 0));
}

void Stack__copy(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));

    set_stack(circa_output(stack, 0), stack_duplicate(self));
}

void Stack__stack_push(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    ca_assert(self != NULL);

    Block* block = as_block(circa_input(stack, 1));

    if (block == NULL)
        return circa_output_error(stack, "Null block for input 1");

    ca_assert(block != NULL);

    Frame* top = vm_push_frame2(self, top_frame(self)->termIndex, vm_compile_block(self, block));

    caValue* inputs = circa_input(stack, 2);

    for (int i=0; i < list_length(inputs); i++) {
        if (i >= frame_register_count(top))
            break;
        copy(list_get(inputs, i), frame_register(top, i));
    }
}

void Stack__stack_pop(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    ca_assert(self != NULL);
    pop_frame(self);
}

void Stack__migrate(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    Block* fromBlock = as_block(circa_input(stack, 1));
    Block* toBlock = as_block(circa_input(stack, 2));
    migrate_stack(self, fromBlock, toBlock);
}

void Stack__migrate_to(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    ca_assert(self != NULL);
    Block* toBlock = as_block(closure_get_block(circa_input(stack, 1)));
    ca_assert(toBlock != NULL);

    migrate_stack(self, frame_block(top_frame(self)), toBlock);
}

void Stack__reset(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    ca_assert(self != NULL);
    stack_reset(self);
}

void Stack__reset_state(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    set_hashtable(&self->state);
}

void Stack__restart(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    ca_assert(self != NULL);
    stack_restart(self);
}

void Stack__restart_discarding_state(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    ca_assert(self != NULL);
    stack_restart_discarding_state(self);
}

void Stack__run(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    ca_assert(self != NULL);

    if (self->caller != NULL)
        return raise_error_msg(self, "Stack already has a call in progress");
    self->caller = stack;

    vm_run(self);

    self->caller = NULL;

    move(circa_input(stack, 0), circa_output(stack, 0));
}

void Stack__frame_from_base(caStack* stack)
{
    Stack* self = (Stack*) get_pointer(circa_input(stack, 0));
    ca_assert(self != NULL);
    int index = circa_int_input(stack, 1);
    if (index >= self->frameCount)
        return circa_output_error(stack, "Index out of range");

    Frame* frame = next_frame_n(first_frame(self), index);
    set_frame_ref(circa_output(stack, 0), frame);
}
void Stack__frame(caStack* stack)
{
    Stack* self = (Stack*) get_pointer(circa_input(stack, 0));
    ca_assert(self != NULL);
    int index = circa_int_input(stack, 1);
    if (index >= self->frameCount)
        return circa_output_error(stack, "Index out of range");

    Frame* frame = prev_frame_n(top_frame(self), index);
    set_frame_ref(circa_output(stack, 0), frame);
}

void Stack__frame_count(caStack* stack)
{
    Stack* self = (Stack*) get_pointer(circa_input(stack, 0));
    set_int(circa_output(stack, 0), stack_frame_count(self));
}

void Stack__output(caStack* stack)
{
    Stack* self = (Stack*) get_pointer(circa_input(stack, 0));
    ca_assert(self != NULL);
    int index = circa_int_input(stack, 1);

    Frame* frame = top_frame(self);
    Term* output = get_output_placeholder(frame_block(frame), index);
    if (output == NULL)
        set_null(circa_output(stack, 0));
    else
        copy(frame_register(frame, output), circa_output(stack, 0));
}

void Stack__errored(caStack* stack)
{
    Stack* self = (Stack*) get_pointer(circa_input(stack, 0));
    set_bool(circa_output(stack, 0), stack_errored(self));
}

void Stack__error_message(caStack* stack)
{
    Stack* self = (Stack*) get_pointer(circa_input(stack, 0));

    Frame* frame = top_frame(self);

    if (frame->termIndex >= frame_register_count(frame)) {
        set_string(circa_output(stack, 0), "");
        return;
    }

    caValue* errorReg = frame_register(frame, frame->termIndex);

    set_string(circa_output(stack, 0), "");
    string_append(circa_output(stack, 0), errorReg);
}

void Stack__toString(caStack* stack)
{
    Stack* self = (Stack*) get_pointer(circa_input(stack, 0));
    ca_assert(self != NULL);

    stack_to_string(self, circa_output(stack, 0), false);
}

void Frame__registers(caStack* stack)
{
    Frame* frame = as_frame_ref(circa_input(stack, 0));
    ca_assert(frame != NULL);

    caValue* out = circa_output(stack, 0);
    int count = frame_register_count(frame);
    set_list(out, count);
    for (int i=0; i < count; i++)
        copy(frame_register(frame, i), list_get(out, i));
}

void Frame__active_value(caStack* stack)
{
    Frame* frame = as_frame_ref(circa_input(stack, 0));
    Term* term = as_term_ref(circa_input(stack, 1));
    caValue* value = stack_find_nonlocal(frame, term);
    if (value == NULL)
        set_null(circa_output(stack, 0));
    else
        set_value(circa_output(stack, 0), value);
}

void Frame__set_active_value(caStack* stack)
{
    Frame* frame = as_frame_ref(circa_input(stack, 0));
    if (frame == NULL)
        return raise_error_msg(stack, "Bad frame reference");

    Term* term = as_term_ref(circa_input(stack, 1));
    caValue* value = stack_find_nonlocal(frame, term);
    if (value == NULL)
        return raise_error_msg(stack, "Value not found");

    set_value(value, circa_input(stack, 2));
}

void Frame__block(caStack* stack)
{
    Frame* frame = as_frame_ref(circa_input(stack, 0));
    ca_assert(frame != NULL);
    set_block(circa_output(stack, 0), frame_block(frame));
}

void Frame__parent(caStack* stack)
{
    Frame* frame = as_frame_ref(circa_input(stack, 0));
    Frame* parent = prev_frame(frame);
    if (parent == NULL)
        set_null(circa_output(stack, 0));
    else
        set_frame_ref(circa_output(stack, 0), parent);
}

void Frame__height(caStack* stack)
{
    Frame* frame = as_frame_ref(circa_input(stack, 0));
    set_int(circa_output(stack, 0), frame_find_index(frame));
}

void Frame__has_parent(caStack* stack)
{
    Frame* frame = as_frame_ref(circa_input(stack, 0));
    Frame* parent = prev_frame(frame);
    set_bool(circa_output(stack, 0), parent != NULL);
}

void Frame__register(caStack* stack)
{
    Frame* frame = as_frame_ref(circa_input(stack, 0));
    ca_assert(frame != NULL);
    int index = circa_int_input(stack, 1);
    copy(frame_register(frame, index), circa_output(stack, 0));
}

void Frame__pc(caStack* stack)
{
    Frame* frame = as_frame_ref(circa_input(stack, 0));
    ca_assert(frame != NULL);
    set_int(circa_output(stack, 0), frame->termIndex);
}

void Frame__parentPc(caStack* stack)
{
    Frame* frame = as_frame_ref(circa_input(stack, 0));
    ca_assert(frame != NULL);
    set_int(circa_output(stack, 0), frame->parentIndex);
}

void Frame__current_term(caStack* stack)
{
    Frame* frame = as_frame_ref(circa_input(stack, 0));
    ca_assert(frame != NULL);
    set_term_ref(circa_output(stack, 0), frame_current_term(frame));
}

#if 0
void Frame__extract_state(caStack* stack)
{
    Frame* frame = as_frame_ref(circa_input(stack, 0));
    caValue* output = circa_output(stack, 0);
    extract_state(frame_block(frame), &frame->state, output);
}
#endif

void stack_install_functions(NativePatch* patch)
{
    circa_patch_function(patch, "Stack.block", Stack__block);
    circa_patch_function(patch, "Stack.dump", Stack__dump);
    circa_patch_function(patch, "Stack.dump_compiled", Stack__dump_compiled);
    circa_patch_function(patch, "Stack.dump_with_bytecode", Stack__dump_with_bytecode);
    circa_patch_function(patch, "Stack.extract_state", Stack__extract_state);
    circa_patch_function(patch, "Stack.eval_on_demand", Stack__eval_on_demand);
    circa_patch_function(patch, "Stack.find_active_value", Stack__find_active_value);
    circa_patch_function(patch, "Stack.find_active_frame_for_term", Stack__find_active_frame_for_term);
    circa_patch_function(patch, "Stack.id", Stack__id);
    circa_patch_function(patch, "Stack.init", Stack__init);
#if 0
    circa_patch_function(patch, "Stack.has_incoming_state", Stack__has_incoming_state);
#endif
    circa_patch_function(patch, "Stack.env", Stack__env);
    circa_patch_function(patch, "Stack.env_map", Stack__env_map);
    circa_patch_function(patch, "Stack.set_env", Stack__set_env);
    circa_patch_function(patch, "Stack.set_env_map", Stack__set_env_map);
    circa_patch_function(patch, "Stack.get_state", Stack__get_state);
    circa_patch_function(patch, "Stack.get_watch_result", Stack__get_watch_result);
    circa_patch_function(patch, "Stack.apply", Stack__call);
    circa_patch_function(patch, "Stack.call", Stack__call);
    circa_patch_function(patch, "Stack.copy", Stack__copy);
    circa_patch_function(patch, "Stack.stack_push", Stack__stack_push);
    circa_patch_function(patch, "Stack.stack_pop", Stack__stack_pop);
    circa_patch_function(patch, "Stack.migrate", Stack__migrate);
    circa_patch_function(patch, "Stack.migrate_to", Stack__migrate_to);
    circa_patch_function(patch, "Stack.reset", Stack__reset);
    circa_patch_function(patch, "Stack.reset_state", Stack__reset_state);
    circa_patch_function(patch, "Stack.restart", Stack__restart);
    circa_patch_function(patch, "Stack.restart_discarding_state", Stack__restart_discarding_state);
    circa_patch_function(patch, "Stack.run", Stack__run);
    circa_patch_function(patch, "Stack.frame", Stack__frame);
    circa_patch_function(patch, "Stack.frame_from_base", Stack__frame_from_base);
    circa_patch_function(patch, "Stack.frame_count", Stack__frame_count);
    circa_patch_function(patch, "Stack.output", Stack__output);
    circa_patch_function(patch, "Stack.errored", Stack__errored);
    circa_patch_function(patch, "Stack.error_message", Stack__error_message);
    circa_patch_function(patch, "Stack.toString", Stack__toString);
    circa_patch_function(patch, "Frame.active_value", Frame__active_value);
    circa_patch_function(patch, "Frame.block", Frame__block);
    circa_patch_function(patch, "Frame.current_term", Frame__current_term);
    circa_patch_function(patch, "Frame.height", Frame__height);
    circa_patch_function(patch, "Frame.has_parent", Frame__has_parent);
    circa_patch_function(patch, "Frame.parent", Frame__parent);
    circa_patch_function(patch, "Frame.register", Frame__register);
    circa_patch_function(patch, "Frame.registers", Frame__registers);
    circa_patch_function(patch, "Frame.pc", Frame__pc);
    circa_patch_function(patch, "Frame.parentIndex", Frame__parentPc);
    circa_patch_function(patch, "Frame.set_active_value", Frame__set_active_value);
#if 0
    circa_patch_function(patch, "Frame.extract_state", Frame__extract_state);
#endif
}

} // namespace circa
