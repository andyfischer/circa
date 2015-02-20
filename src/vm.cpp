// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "bytecode2.h"
#include "blob.h"
#include "block.h"
#include "closures.h"
#include "hashtable.h"
#include "kernel.h"
#include "list.h"
#include "names.h"
#include "symbols.h"
#include "tagged_value.h"
#include "type.h"
#include "vm.h"
#include "world.h"

#define TRACE_STATE_EXECUTION 0

namespace circa {

VM* new_vm(Block* main)
{
    ca_assert(main->world != NULL);

    VM* vm = (VM*) malloc(sizeof(VM));
    vm->pc = 0;
    vm->world = main->world;
    vm->id = vm->world->nextStackID++;
    vm->mainBlock = main;
    initialize_null(&vm->topLevelUpvalues);
    vm->bc = NULL;
    initialize_null(&vm->bcHacks);
    vm->noSaveState = false;
    vm->noEffect = false;
    initialize_null(&vm->termOverrides);
    set_hashtable(&vm->termOverrides);
    vm->error = false;
    vm->stackTop = 0;
    vm->stack.init();
    initialize_null(&vm->state);
    vm->stateTop = -1;
    set_hashtable(&vm->state);
    vm->inputCount = 0;
    initialize_null(&vm->demandEvalMap);
    set_hashtable(&vm->demandEvalMap);
    initialize_null(&vm->incomingUpvalues);
    initialize_null(&vm->incomingEnv);
    set_hashtable(&vm->incomingEnv);
    initialize_null(&vm->env);
    set_hashtable(&vm->env);
    rand_init(&vm->randState, 0);
    return vm;
}

void free_vm(VM* vm)
{
    set_null(&vm->topLevelUpvalues);
    set_null(&vm->bcHacks);
    vm->stack.clear();
    set_null(&vm->incomingUpvalues);
    set_null(&vm->state);
    set_null(&vm->demandEvalMap);
    set_null(&vm->incomingEnv);
    set_null(&vm->env);
}

VM* vm_duplicate(VM* source)
{
    VM* vm = new_vm(source->mainBlock);
    copy(&source->env, &vm->env);
    vm_set_state(vm, vm_get_state(source));
    return vm;
}

static inline Value* get_slot(VM* vm, int slot)
{
    int index = vm->stackTop + slot;
    return vm->stack[index];
}

void vm_grow_stack(VM* vm, int newSize)
{
    vm->stack.reserve(newSize);
}

void make_vm(VM* vm)
{
    Value* closure = vm->input(0);
    Block* block = func_block(closure);
    Value* upvalues = func_bindings(closure);

    VM* newVm = new_vm(block);
    copy(upvalues, &newVm->topLevelUpvalues);

    if (block == NULL)
        return vm->throw_str("NULL block");

    Value* out = vm->output();
    make(TYPES.vm, out);
    set_pointer(out, newVm);
}

void VM__call(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    Value* inputs = vm->input(1);

    vm_grow_stack(self, inputs->length() + 1);

    for (int i=0; i < inputs->length(); i++)
        move(inputs->index(i), self->input(i));

    vm_run(self, vm);

    copy(self->output(), vm->output());
}

void VM__copy(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    VM* dupe = vm_duplicate(self);
    Value* out = vm->output();
    make(TYPES.vm, out);
    set_pointer(out, dupe);
}

void VM__dump(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    self->dump();
}

void VM__errored(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    vm->output()->set_bool(self->error);
}

void VM__get_state(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    copy(&self->state, vm->output());
}

void VM__id(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    set_int(vm->output(), self->id);
}

void VM__set_state(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    Value* state = vm->input(1);
    vm_set_state(self, state);
}

void VM__frame_list(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    vm_to_frame_list(self, vm->output());
}

void VM__slot(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    int index = vm->input(1)->as_i();
    copy(get_slot(self, index), vm->output());
}

void VM__env_map(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    copy(&self->incomingEnv, vm->output());
}

void VM__set_env(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    Value* key = vm->input(1);
    Value* val = vm->input(2);
    copy(val, hashtable_insert(&self->incomingEnv, key));
}

void VM__set_env_map(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    Value* map = vm->input(1);
    copy(map, &self->incomingEnv);
}

void VM__get_raw_slots(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    Value* out = vm->output();
    out->set_list(self->stack.size);
    for (int i=0; i < self->stack.size; i++)
        copy(self->stack[i], out->index(i));
}

void VM__get_raw_ops(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    if (self->bc == NULL)
        set_blob_alloc_raw(vm->output(), NULL, 0);
    else
        set_blob_alloc_raw(vm->output(), (char*) self->bc->ops, self->bc->opCount * sizeof(Op));
}

void VM__get_raw_mops(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    if (self->bc == NULL)
        set_blob_alloc_raw(vm->output(), NULL, 0);
    else
        set_blob_alloc_raw(vm->output(), (char*) self->bc->metadata,
            self->bc->metadataSize * sizeof(BytecodeMetadata));
}

void VM__get_bytecode_const(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    int index = vm->input(1)->as_i();
    copy(get_const(self->bc, index), vm->output());
}

void VM__precompile(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    Block* block = vm->input(1)->asBlock();
    vm_prepare_bytecode(self, vm);
    find_or_compile_major_block(self->bc, block);
}

void bytecode_get_mop_size(VM* vm)
{
    set_int(vm->output(), (int) sizeof(BytecodeMetadata));
}

void VM__trace_state_path(VM* vm)
{
    VM* self = (VM*) get_pointer(vm->input(0));
    
    int top = vm->stackTop;
    int addr = vm->pc;
}

void get_env(VM* vm)
{
    Value* value = hashtable_get(&vm->env, vm->input(0));
    if (value != NULL)
        copy(value, vm->output());
    else
        set_null(vm->output());
}

VMStackFrame vm_top_stack_frame(VM* vm)
{
    VMStackFrame out;
    out.top = vm->stackTop;
    out.pc = vm->pc;
    return out;
}

VMStackFrame vm_walk_up_stack_frames(VM* vm, VMStackFrame frame, int count)
{
    while (count > 0) {
        if (frame.top < 2) {
            frame.top = -1;
            frame.pc = -1;
            return frame;
        }

        frame.pc = vm->stack[frame.top - 2]->as_i() - 1;
        frame.top = vm->stack[frame.top - 1]->as_i();
        count--;
    }
    return frame;
}

void reflect_caller(VM* vm)
{
    int height = vm->input(0)->as_i();
    VMStackFrame frame = vm_walk_up_stack_frames(vm, vm_top_stack_frame(vm), height + 1);

    if (frame.top == -1)
        return set_null(vm->output());

    Term* term = find_active_term(vm->bc, frame.pc);
    set_term_ref(vm->output(), term);
}

void vm_demand_eval_find_existing(VM* vm)
{
    Term* term = vm->input(0)->asTerm();
    Value* found = hashtable_get_term_key(&vm->demandEvalMap, term);
    if (found == NULL)
        set_list(vm->output());
    else
        set_list_1(vm->output(), found);
}

void vm_demand_eval_store(VM* vm)
{
    Term* term = vm->input(0)->asTerm();
    copy(vm->input(1), hashtable_insert_term_key(&vm->demandEvalMap, term));
}

#if 0
VMStateFrame find_state_frame_at_maddr(VM* vm, int top, int maddr)
{
    VMStateFrame stateFrame;
    stateFrame.top = top;
    stateFrame.maddr = 0;
    stateFrame.key = NULL;
    stateFrame.incoming = NULL;
    stateFrame.outgoing = NULL;
    stateFrame.isMajorBlock = false;

    bool done = false;
    for (; !done && maddr >= 0; maddr--) {
        maddr = mop_search_skip_minor_blocks(vm->bc, maddr);

        BytecodeMetadata md = vm->bc->metadata[maddr];
        switch (md.mopcode) {
        case mop_state_key:
            stateFrame.key = vm->stack[top + md.slot];
            break;
        case mop_state_header:
            stateFrame.incoming = vm->stack[top + md.slot];
            stateFrame.outgoing = vm->stack[top + md.slot + 1];
            break;
        case mop_minor_block_start:
            stateFrame.maddr = maddr;
            done = true;
            break;
        case mop_major_block_start:
            stateFrame.isMajorBlock = true;
            done = true;
            break;
        }
    }

    if (stateFrame.key == NULL && top > 0) {
        // No mop_state_key header found, use the calling term as state key.
        VMStackFrame stackFrame;
        stackFrame.top = top;
        stackFrame = vm_walk_up_stack_frames(vm, stackFrame, 1);
        Term* term = find_active_term(vm->bc, stackFrame.pc);
        stateFrame.key = unique_name(term);
    }

    return stateFrame;
}

VMStateFrame find_state_frame_at_stack_frame(VM* vm, VMStackFrame stackFrame)
{
    int maddr = find_metadata_addr_for_addr(vm->bc, stackFrame.pc);
    return find_state_frame_at_maddr(vm, stackFrame.top, maddr);
}

VMStateFrame find_parent_state_frame(VM* vm, VMStateFrame frame)
{
    if (frame.isMajorBlock) {
        if (frame.top == 0) {
            VMStateFrame result;
            result.top = -1;
            return result;
        }

        VMStackFrame stackFrame;
        stackFrame.top = frame.top;

        stackFrame = vm_walk_up_stack_frames(vm, stackFrame, 1);
        return find_state_frame_at_stack_frame(vm, stackFrame);
    } else {
        return find_state_frame_at_maddr(vm, frame.top, frame.maddr - 1);
    }
}
#endif

void push_state_frame(VM* vm, int newTop, Value* key)
{
    #if TRACE_STATE_EXECUTION
        printf("pushing state frame, top = %d (was %d), key = %s\n", newTop,
            vm->stateTop, key->to_c_string());
    #endif

    Value* frameRetTop = vm->stack[newTop];
    Value* frameKey = vm->stack[newTop + 1];
    Value* frameIncoming = vm->stack[newTop + 2];
    Value* frameOutgoing = vm->stack[newTop + 3];

    set_int(frameRetTop, vm->stateTop);

    if (vm->stateTop == -1) {
        copy(&vm->state, frameIncoming);
        set_null(frameKey);
        #if TRACE_STATE_EXECUTION
            printf("  top frame, copied vm->state: %s\n", frameIncoming->to_c_string());
        #endif
    } else {
        copy(key, frameKey);
        Value* parentIncoming = vm->stack[vm->stateTop + 2];
        #if TRACE_STATE_EXECUTION
            printf("  parent state frame is: %s\n", parentIncoming->to_c_string());
        #endif
        Value* found = NULL;
        if (is_hashtable(parentIncoming))
            found = hashtable_get(parentIncoming, key);
        if (found != NULL) {
            copy(found, frameIncoming);

            #if TRACE_STATE_EXECUTION
                printf("  incoming state frame is: %s\n", frameIncoming->to_c_string());
            #endif
        } else {
            set_hashtable(frameIncoming);
            #if TRACE_STATE_EXECUTION
                printf("  incoming state frame is null\n");
            #endif
        }
    }

    set_null(frameOutgoing);

    vm->stateTop = newTop;
}

void pop_state_frame(VM* vm)
{
    int top = vm->stateTop;
    ca_assert(top >= 0);

    Value* frameRetTop = vm->stack[top];
    Value* frameKey = vm->stack[top + 1];
    Value* frameIncoming = vm->stack[top + 2];
    Value* frameOutgoing = vm->stack[top + 3];

    int retTop = frameRetTop->as_i();

    #if TRACE_STATE_EXECUTION
        printf("popping state frame, top = %d (was %d), key = %s\n", retTop,
            vm->stateTop, frameKey->to_c_string());
        printf("  frameOutgoing is %s\n", frameOutgoing->to_c_string());
    #endif

    if (retTop == -1) {
        move(frameOutgoing, &vm->state);
        #if TRACE_STATE_EXECUTION
            printf("  saved to vm->state: %s\n", vm->state.to_c_string());
        #endif
    } else {
        Value* parentOutgoing = vm->stack[retTop + 3];
        if (is_null(frameOutgoing)) {
            if (is_hashtable(parentOutgoing))
                hashtable_remove(parentOutgoing, frameKey);
            #if TRACE_STATE_EXECUTION
                printf("  deleted key from parent\n");
            #endif
        } else {
            if (!is_hashtable(parentOutgoing))
                set_hashtable(parentOutgoing);
            move(frameOutgoing, hashtable_insert(parentOutgoing, frameKey));
        }
    }

    vm->stateTop = retTop;
}

void pop_discard_state_frame(VM* vm)
{
    int top = vm->stateTop;
    Value* frameRetTop = vm->stack[top];
    vm->stateTop = frameRetTop->as_i();
}

void get_state_value(VM* vm, Value* key, Value* dest)
{
    ca_assert(vm->stateTop >= 0);

    Value* frameIncoming = vm->stack[vm->stateTop + 2];
    Value* found = NULL;
    if (is_hashtable(frameIncoming))
        found = hashtable_get(frameIncoming, key);
    if (found == NULL)
        set_null(dest);
    else
        copy(found, dest);

    #if TRACE_STATE_EXECUTION
        printf("get state value, key = %s, found = %s\n", key->to_c_string(), dest->to_c_string());
    #endif
}

void save_state_value(VM* vm, Value* key, Value* value)
{
    #if TRACE_STATE_EXECUTION
        printf("saving state value, key = %s, value = %s\n", key->to_c_string(),
            value->to_c_string());
    #endif

    ca_assert(vm->stateTop >= 0);

    Value* frameOutgoing = vm->stack[vm->stateTop + 3];
    if (is_null(value)) {
        if (is_hashtable(frameOutgoing))
            hashtable_remove(frameOutgoing, key);
    } else {
        if (!is_hashtable(frameOutgoing))
            set_hashtable(frameOutgoing);
        copy(value, hashtable_insert(frameOutgoing, key));
    }
}

#if 0
Value* open_incoming_state(VM* vm, VMStateFrame frame)
{
    #if TRACE_STATE_EXECUTION
        printf("open_incoming_state: state frame {top:%d, maddr:%d}\n", frame.top, frame.maddr);
    #endif

    if (frame.incoming != NULL && is_hashtable(frame.incoming)) {
        // Header found and value already loaded.
        #if TRACE_STATE_EXECUTION
            printf("open_incoming_state: found loaded incoming %s\n",
                frame.incoming->to_c_string());
        #endif
        return frame.incoming;
    }

    VMStateFrame parentFrame = find_parent_state_frame(vm, frame);
    
    #if TRACE_STATE_EXECUTION
        printf("open_incoming_state: found parent frame {top:%d, maddr:%d}\n",
            parentFrame.top, parentFrame.maddr);
    #endif

    if (parentFrame.top == -1) {
        #if TRACE_STATE_EXECUTION
            printf("open_incoming_state: using topmost frame\n");
        #endif
        // Reached the top frame.
        return &vm->state;
    }

    // Pull value out from parent frame
    Value* parentIncoming = open_incoming_state(vm, parentFrame);

    #if TRACE_STATE_EXECUTION
        printf("open_incoming_state: parent incoming is: %s\n", parentIncoming->to_c_string());
    #endif

    Value* found = NULL;
    
    if (parentIncoming != NULL)
        found = hashtable_get(parentIncoming, frame.key);

    #if TRACE_STATE_EXECUTION
        printf("open_incoming_state: found is: %s\n", found->to_c_string());
    #endif

    if (found == NULL) {
        if (frame.incoming != NULL) {
            // Save an empty hashtable in our header, so that we don't need to do this search again.
            set_hashtable(frame.incoming);
            return frame.incoming;
        }
        return NULL;
    }
    
    // Found the result, save to our header and return.
    if (frame.incoming != NULL)
        copy(found, frame.incoming); // Future: might be able to optimize this to move()

    return found;
}

Value* prepare_outgoing_state(VM* vm, VMStateFrame frame)
{
    if (frame.outgoing != NULL && is_hashtable(frame.outgoing))
        // Header found and outgoing value already prepared
        return frame.outgoing;

    if (frame.outgoing != NULL) {
        set_hashtable(frame.outgoing);
        return frame.outgoing;
    }

    // No state header, add a field to parent state.
    VMStateFrame parentFrame = find_parent_state_frame(vm, frame);

    if (parentFrame.top == -1)
        // Reached the top frame.
        return &vm->state;

    Value* parentOutgoing = prepare_outgoing_state(vm, parentFrame);

    Value* outgoing = hashtable_insert(parentOutgoing, frame.key);
    set_hashtable(outgoing);
    return outgoing;
}

void vm_incoming_state(VM* vm)
{

    int height = vm->input(0)->as_i();
    VMStackFrame stackFrame = vm_walk_up_stack_frames(vm, vm_top_stack_frame(vm), height + 1);

    #if TRACE_STATE_EXECUTION
        printf("vm_incoming_state: stack frame {top:%d, pc:%d}\n", stackFrame.top, stackFrame.pc);
    #endif

    Value* value = open_incoming_state(vm, find_state_frame_at_stack_frame(vm, stackFrame));
    if (value == NULL)
        set_hashtable(vm->output());
    else
        copy(value, vm->output());
}

void vm_save_declared_state(VM* vm)
{
    Value* result = vm->input(0);
    VMStackFrame frame = vm_walk_up_stack_frames(vm, vm_top_stack_frame(vm), 1);
    Term* caller = find_active_term(vm->bc, frame.pc);
    ca_assert(caller != NULL);
    Value* stateKey = unique_name(caller);
    Value* outgoing = prepare_outgoing_state(vm, find_state_frame_at_stack_frame(vm, frame));

    #if TRACE_STATE_EXECUTION
        printf("vm_save_declared_state: saving %s with key %s to outgoing %s\n",
            result->to_c_string(), stateKey->to_c_string(),
            outgoing->to_c_string());
    #endif

    copy(result, hashtable_insert(outgoing, stateKey));
}

void vm_close_stateful_minor_frame(VM* vm)
{
    VMStackFrame stackFrame = vm_walk_up_stack_frames(vm, vm_top_stack_frame(vm), 1);
    VMStateFrame stateFrame = find_state_frame_at_stack_frame(vm, stackFrame);

    // should always be a state header where there is a vm_close_stateful_minor_frame()
    ca_assert(stateFrame.incoming != NULL);

    VMStateFrame parentStateFrame = find_parent_state_frame(vm, stateFrame);

    #if TRACE_STATE_EXECUTION
        printf("vm_close_stateful_minor_frame: saving outgoing %s with key %s\n",
            stateFrame.outgoing->to_c_string(), stateFrame.key->to_c_string());
    #endif

    if (parentStateFrame.top == -1) {
        move(stateFrame.outgoing, &vm->state);
    } else {
        Value* parentOutgoing = prepare_outgoing_state(vm, parentStateFrame);
        move(stateFrame.outgoing, hashtable_insert(parentOutgoing, stateFrame.key));
    }
}

void vm_capture_state_frames(VM* vm)
{
    Value* out = vm->output()->set_list(0);

    VMStateFrame frame = find_state_frame_at_stack_frame(vm, vm_top_stack_frame(vm));
    while (frame.top >= 0) {

        Value* item = out->append();
        set_hashtable(item);
        item->insert(s_top)->set_int(frame.top);
        item->insert(s_maddr)->set_int(frame.maddr);
        if (frame.key != NULL)
            item->insert(s_key)->set(frame.key);
            /*
        if (frame.incoming != NULL)
            item->insert(s_incoming)->set(frame.incoming);
        if (frame.outgoing != NULL)
            item->insert(s_outgoing)->set(frame.outgoing);
            */

        frame = find_parent_state_frame(vm, frame);
    }
}
#endif

Value* VM::input(int index)
{
    return get_slot(this, 1 + index);
}

Value* VM::output()
{
    return get_slot(this, 0);
}

void vm_to_frame_list(VM* vm, Value* frameList)
{
    Bytecode* bc = vm->bc;

    set_list(frameList, 0);

    int currentTop = vm->stackTop;
    int currentPc = vm->pc;

    while (true) {
        Value* frame = frameList->append()->set_hashtable();

        Block* block = find_active_major_block(bc, currentPc);

        //Value blockVal;
        //blockVal.set_block(block);
        //Value* blockInfo = bc->blockToInfo.val_key(&blockVal);
        //int slotCount = blockInfo->field(s_slotCount)->as_i();

        Term* currentTerm = find_active_term(bc, currentPc);

        frame->insert(s_block)->set_block(block);
        frame->insert(s_top)->set_int(currentTop);
        frame->insert(s_pc)->set_int(currentPc);
        //frame->insert(s_slotCount)->set_int(slotCount);
        frame->insert(s_current_term)->set_term(currentTerm);

        if (currentTop <= 0)
            break;

        currentPc = vm->stack[currentTop - 2]->as_i();
        int nextTop = vm->stack[currentTop - 1]->as_i();

        if (nextTop >= currentTop)
            internal_error("vm_to_frame_list: malformed stack, ret_top >= top");

        currentTop = nextTop;
    }

    // Reverse so that 'bottom' frames are first.
    list_reverse(frameList);
}

Term* vm_calling_term(VM* vm)
{
    if (vm->stackTop == 0)
        return NULL;
    VMStackFrame stackFrame;
    stackFrame.top = vm->stackTop;
    stackFrame = vm_walk_up_stack_frames(vm, stackFrame, 1);
    return find_active_term(vm->bc, stackFrame.pc);
}

void VM::dump()
{
    VM* vm = this;
    Value value;
    Value* frames = &value;
    vm_to_frame_list(vm, frames);

    printf("[Stack]\n");

    for (int frameIndex = 0; frameIndex < frames->length(); frameIndex++) {
        Value* frame = frames->index(frameIndex);
        bool isTopFrame = frameIndex == (frames->length() - 1);
        int top = frame->field(s_top)->as_i();

        Term* currentTerm = frame->field(s_current_term)->asTerm();
        Block* block = frame->field(s_block)->asBlock();

        printf(" [Frame %d, Block#%d '%s', top %d]\n", frameIndex,
            block->id, block->name(),
            top);

        printf("  pc: %d\n", frame->field(s_pc)->as_i());

        if (isTopFrame)
            printf("  inputCount: %d\n", vm->inputCount);
        
        if (currentTerm != NULL)
            printf("  currentTerm: #%d '%s'\n", currentTerm->id, currentTerm->name());
    }
}

void VM::throw_error(Value* err)
{
    this->error = true;
    copy(err, this->output());
}

void VM::throw_str(const char* str)
{
    Value val;
    set_string(&val, str);
    throw_error(&val);
}

bool vm_has_error(VM* vm)
{
    return vm->error;
}

Value* vm_get_error(VM* vm)
{
    if (!vm->error)
        return NULL;

    return get_slot(vm, 0);
}

Value* vm_get_state(VM* vm)
{
    return &vm->state;
}

void vm_set_state(VM* vm, Value* state)
{
    copy(state, &vm->state);
}

void vm_on_code_change(VM* vm)
{
    free_bytecode(vm->bc);
    vm->bc = NULL;
}

bool vm_check_if_hacks_changed(VM* vm)
{
    Value* foundHacks = hashtable_get_symbol_key(&vm->env, s_hacks);

    if (foundHacks == NULL) {
        if (!is_null(&vm->bcHacks)) {
            set_null(&vm->bcHacks);
            return true;
        }
        return false;
    }

    if (!equals(&vm->bcHacks, foundHacks)) {
        copy(foundHacks, &vm->bcHacks);

        return true;
    }
    return false;
}

void vm_update_derived_hack_info(VM* vm)
{
    vm->noSaveState = false;
    vm->noEffect = false;
    set_hashtable(&vm->termOverrides);

    Value* hacks = &vm->bcHacks;

    if (!is_list(hacks))
        return;

    for (int i=0; i < hacks->length(); i++) {
        Value* element = hacks->index(i);
        if (symbol_eq(element, s_no_effect))
            vm->noEffect = true;
        else if (symbol_eq(element, s_no_save_state))
            vm->noSaveState = true;
        else if (is_list_with_length(element, 3) && symbol_eq(element->index(0), s_set_value))
            copy(element->index(2), vm->termOverrides.insert_val(element->index(1)));
    }
}

void vm_prepare_env(VM* vm, VM* callingVM)
{
    copy(&vm->incomingEnv, &vm->env);

    Value* myEnv = &vm->env;

    if (callingVM != NULL) {
        Value* callingEnv = &callingVM->env;
        touch(myEnv);

        for (HashtableIterator it(callingEnv); it; ++it) {
            Value* existing = hashtable_get(myEnv, it.key());
            if (existing == NULL) 
                copy(it.value(), hashtable_insert(myEnv, it.key()));
        }
    }
}

void vm_cleanup_on_stop(VM* vm)
{
    for (int i=1; i < vm->stack.size; i++)
        set_null(vm->stack[i]);
    vm->inputCount = 0;
    set_null(&vm->incomingUpvalues);
}

#if 0
Value* vm_state_push_key(VM* vm)
{
    vm->stateStackTop += 2;
    int top = vm->stateStackTop;
    vm->stateStack.reserve(top + 1);
    Value* topKey = vm->stateStack[top - 1];
    Value* topVal = vm->stateStack[top];
    return topKey;
}

void vm_state_load_frame(VM* vm, int top)
{
    vm->stateStack.reserve(top + 1);
    Value* val = vm->stateStack[top];

    if (top == 0)
        return;
    if (!is_null(val))
        return;

    vm_state_load_frame(vm, top - 2);

    Value* key = vm->stateStack[top - 1];
    Value* parentVal = vm->stateStack[top - 2];

    if (!is_null(parentVal)) {
        Value* found = hashtable_get(parentVal, key);

        // TODO: Can probably make this a move()
        if (found != NULL)
            copy(found, val);
    }
}

void vm_state_store_frame(VM* vm)
{
    int top = vm->stateStackTop;
    if (top == 0)
        return;

    vm->stateStackTop -= 2;

    Value* val = vm->stateStack[top];
    Value* key = vm->stateStack[top-1];
    Value* parentVal = vm->stateStack[top-2];

    if (vm->noSaveState) {
        set_null(val);
        set_null(key);
        return;
    }

    if (is_null(val)) {
        if (!is_null(parentVal))
            hashtable_remove(parentVal, key);
        return;
    }

    if (is_null(parentVal))
        set_hashtable(parentVal);

    move(val, hashtable_insert(parentVal, key));
    set_null(key);
}
#endif

int find_state_slot(VM* vm, int addr)
{
    return -1;
}

Value* prepare_outgoing_state_frame_for_save(VM* vm)
{
    return NULL;
}

Value* circa_input(VM* vm, int index)
{
    return vm->input(index);
}

Value* circa_output(VM* vm)
{
    return vm->output();
}

void circa_throw(VM* vm, const char* msg)
{
    Value str;
    set_string(&str, msg);
    vm->throw_error(&str);
}

void vm_install_functions(NativePatch* patch)
{
    circa_patch_function2(patch, "make_vm", make_vm);
    circa_patch_function2(patch, "VM.call", VM__call);
    circa_patch_function2(patch, "VM.copy", VM__copy);
    circa_patch_function2(patch, "VM.dump", VM__dump);
    circa_patch_function2(patch, "VM.errored", VM__errored);
    circa_patch_function2(patch, "VM.get_state", VM__get_state);
    circa_patch_function2(patch, "VM.id", VM__id);
    circa_patch_function2(patch, "VM.set_state", VM__set_state);
    circa_patch_function2(patch, "VM.frame_list", VM__frame_list);
    circa_patch_function2(patch, "VM.slot", VM__slot);
    circa_patch_function2(patch, "VM.env_map", VM__env_map);
    circa_patch_function2(patch, "VM.set_env", VM__set_env);
    circa_patch_function2(patch, "VM.set_env_map", VM__set_env_map);
    circa_patch_function2(patch, "VM.get_raw_slots", VM__get_raw_slots);
    circa_patch_function2(patch, "VM.get_raw_ops", VM__get_raw_ops);
    circa_patch_function2(patch, "VM.get_raw_mops", VM__get_raw_mops);
    circa_patch_function2(patch, "VM.get_bytecode_const", VM__get_bytecode_const);
    circa_patch_function2(patch, "VM.precompile", VM__precompile);
    circa_patch_function2(patch, "bytecode_mop_size", bytecode_get_mop_size);
    circa_patch_function2(patch, "reflect_caller", reflect_caller);
    circa_patch_function2(patch, "vm_demand_eval_find_existing", vm_demand_eval_find_existing);
    circa_patch_function2(patch, "vm_demand_eval_store", vm_demand_eval_store);
#if 0
    circa_patch_function2(patch, "vm_incoming_state", vm_incoming_state);
    circa_patch_function2(patch, "vm_save_declared_state", vm_save_declared_state);
    circa_patch_function2(patch, "vm_close_stateful_minor_frame", vm_close_stateful_minor_frame);
    circa_patch_function2(patch, "vm_capture_state_frames", vm_capture_state_frames);
#endif
    circa_patch_function2(patch, "env", get_env);
}

void vm_setup_type(Type* type)
{
    set_string(&type->name, "VM");
}

} // namespace circa
