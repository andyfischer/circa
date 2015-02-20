// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "rand.h"
#include "value_array.h"

namespace circa {

struct VM {
    World* world;
    int id;

    u32 pc;
    Block* mainBlock;
    Value topLevelUpvalues;
    Bytecode* bc;
    
    // Cached :hacks value from env
    Value bcHacks;

    // Derived contents of bcHacks:
    bool noSaveState;
    bool noEffect;
    Value termOverrides;

    bool error;

    u16 stackTop;
    ValueArray stack;
    u8 inputCount;
    Value incomingUpvalues;

    Value state;
    int stateTop;

    Value demandEvalMap; // Map of Term -> value

    Value incomingEnv; // env from calling context
    Value env; // actual env (including inheritance)
    RandState randState;

    Value* input(int index);
    Value* output();
    void throw_error(Value* err);
    void throw_str(const char* str);
    void dump();
    bool has_error() { return error; }
};

struct VMStackFrame {
    int top;
    int pc;
};

#if 0
struct VMStateFrame {
    int top;
    int maddr;
    Value* key;
    Value* incoming;
    Value* outgoing;
    bool isMajorBlock;
};
#endif
    
VM* new_vm(Block* main);
void free_vm(VM* vm);
void vm_grow_stack(VM* vm, int newSize);
void vm_on_code_change(VM* vm);
bool vm_check_if_hacks_changed(VM* vm);
void vm_update_derived_hack_info(VM* vm);
bool vm_has_error(VM* vm);
Value* vm_get_error(VM* vm);
Value* vm_get_state(VM* vm);
void vm_set_state(VM* vm, Value* state);

void push_state_frame(VM* vm, int newTop, Value* key);
void pop_state_frame(VM* vm);
void pop_discard_state_frame(VM* vm);
void get_state_value(VM* vm, Value* key, Value* dest);
void save_state_value(VM* vm, Value* key, Value* value);

void vm_prepare_env(VM* vm, VM* callingVM);
void vm_cleanup_on_stop(VM* vm);

Term* vm_calling_term(VM* vm);
void vm_to_frame_list(VM* vm, Value* frameList);

Value* circa_input(VM* vm, int index);
Value* circa_output(VM* vm);
void circa_throw(VM* vm, const char* msg);

void vm_install_functions(NativePatch* patch);
void vm_setup_type(Type* type);

} // namespace circa
