// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#ifndef CIRCA_H_INCLUDED
#define CIRCA_H_INCLUDED

#ifdef _MSC_VER
    // No stdbool on Windows
#else
    #include <stdbool.h>
#endif

// Public API

#ifdef __cplusplus
extern "C" {
#endif

// -- Circa Types --

#ifdef __cplusplus
namespace circa {
    struct Actor;
    struct ActorSpace;
    struct Block;
    struct Stack;
    struct Term;
    struct Type;
    struct World;
    struct NativePatch;
}

typedef circa::Actor caActor;
typedef circa::ActorSpace caActorSpace;

typedef circa::Block caBlock;

// a Stack holds the interpreter's current state, including a list of frames (activation
// records). Each Stack corresponds to one lightweight thread (not OS thread).
typedef circa::Stack caStack;

typedef circa::Term caTerm;

// a Type holds data for a single Circa type, including a name, handlers for
// initialization and destruction, and various other handlers and metadata.
typedef circa::Type caType;

// a World holds miscellaneous shared runtime information. A process should create one
// World that is used across the program.
typedef circa::World caWorld;

typedef circa::NativePatch caNativePatch;

#else

typedef struct caBlock caBlock;
typedef struct caStack caStack;
typedef struct caTerm caTerm;
typedef struct caType caType;
typedef struct caWorld caWorld;
typedef struct caNativePatch caNativePatch;

#endif

union caValueData {
    int asint;
    float asfloat;
    bool asbool;
    void* ptr;
};

// a Value is a variant value. It holds two pointers, one pointer to the Type object and
// one to the value's data. For some types (such as integers, floats, booleans), the data
// section holds the actual value and not a pointer.
//
// If you allocate caValue yourself then you must call circa_init_value before using it,
// and call circa_set_null before deallocating it.
struct caValue
{
    caValueData value_data;
    caType* value_type;

#ifdef __cplusplus
    // For debugging:
    void dump();

protected:
    // Don't use caValue as a C++ type (use circa::Value instead)
    caValue() {}
    ~caValue() {}

private:
    caValue(caValue const&);
    caValue& operator=(caValue const&);
#endif
};

#ifdef __cplusplus

// C++ wrapper on caValue. Provides C++-style initialization and destruction.
namespace circa {

struct Value : caValue
{
    Value();
    ~Value();
    Value(Value const&);
    Value& operator=(Value const&);
};

} // namespace circa

#endif // __cplusplus

// a Function holds data for a single Circa function, including a name, the function's
// definition (stored as a Block), and various other metadata. Each Function has an
// EvaluateFunc which is triggered when the function is called.
typedef struct caFunction caFunction;

// a Symbol is an alias for an integer, used to reference an interned string.
typedef int caName;

// EvaluateFunc is the signature for a C evaluation function. The function will access the
// stack to read inputs (if any), possibly perform some action, and write output values
// (if any) back to the stack.
typedef void (*caEvaluateFunc)(caStack* stack);

// ReleaseFunc is the signature for a C function that runs when a user object is about to
// be destroyed. This is used for 'handle' values.
typedef void (*caReleaseFunc)(caValue* value);

// -- Setting up the Circa environment --

// Create a caWorld. This should be done once at the start of the program.
caWorld* circa_initialize();

// Uninitialize a caWorld. This call is entirely optional, but using it will make memory-
// leak checking tools happier.
void circa_shutdown(caWorld*);

// Add a module search path. This is used when processing 'import' statements.
void circa_add_module_search_path(caWorld* world, const char* path);

// Load a module by opening the given filename as a source file.
caBlock* circa_load_module_from_file(caWorld* world,
                                      const char* module_name,
                                      const char* filename);
caBlock* circa_load_module(caWorld* world, const char* moduleName);

// -- Controlling Actors --
#if 0 // disabled as of actors v3
void circa_actor_new_from_file(caWorld* world, const char* actorName, const char* filename);
caValue* circa_actor_new_from_module(caWorld* world, const char* actorName, const char* moduleName);
void circa_actor_post_message(caWorld* world, const char* actorName, caValue* message);
void circa_actor_run_message(caWorld* world, const char* actorName, caValue* message);
int circa_actor_run_queue(caWorld* world, const char* actorName, int maxMessages);
int circa_actor_run_all_queues(caWorld* world, int maxMessages);
void circa_actor_clear_all(caWorld* world);
#endif

// -- Controlling the Interpreter --

// Allocate a new Stack object.
caStack* circa_create_stack(caWorld* world);

// Deallocate a Stack object.
void circa_free_stack(caStack* stack);

// Run a whole module.
void circa_run_module(caStack* stack, const char* moduleName);

// Push a function to the stack.
void circa_push_function(caStack* stack, caBlock* func);

// Find a function by name and push it to the stack. Returns whether the function
// name was found.
bool circa_push_function_by_name(caStack* stack, const char* name);

// Push a module to the stack by name.
void circa_push_module(caStack* stack, const char* name);

// Refresh all loaded modules (if the source file has changed, load the latest file)
//void circa_refresh_all_modules(caWorld* world);
void circa_update_changed_files(caWorld* world);

// Run the current stack.
void circa_run(caStack* stack);

void circa_pop(caStack* stack);

void circa_call_method(caStack* stack, const char* funcName, caValue* object, caValue* ins, caValue* outs);

// Signal that an error has occurred.
void circa_output_error(caStack* stack, const char* msg);

// Return whether a runtime error occurred.
bool circa_has_error(caStack* stack);

// Clear all frames from a Stack.
void circa_clear_stack(caStack* stack);

// Clear all frames but the topmost, rewind the PC, and clear temporary values.
void circa_restart(caStack* stack);

// Print a human-readable description of the stack's error to stdout.
void circa_print_error_to_stdout(caStack* stack);

// -- Fetching Inputs & Outputs --

// Get the number of inputs.
int circa_num_inputs(caStack* stack);

// Retrieve the given input value. This value may not be modified.
caValue* circa_input(caStack* stack, int index);

// Read the given input index as an integer.
// Equivalent to: circa_int(circa_input(stack, index))
int circa_int_input(caStack* stack, int index);

// Read the given input index as a string
// Equivalent to: circa_string(circa_input(stack, index))
const char* circa_string_input(caStack* stack, int index);

// Read the given input index as a float
// Equivalent to: circa_to_float(circa_input(stack, index))
// (Note that we use circa_to_float, not circa_float)
float circa_float_input(caStack* stack, int index);

// Read the given input index as a bool
// Equivalent to: circa_bool(circa_input(stack, index))
float circa_bool_input(caStack* stack, int index);

// Fetch the given output value. This value should be modified.
caValue* circa_output(caStack* stack, int index);

// Initialize the given output value, using the default create() function on the
// Term's declared type. Also, returns the output value for convenient writing.
// This function is unnecessary if you write to the output using one of the
// circa_set_XXX functions.
caValue* circa_create_default_output(caStack* stack, int index);

// Fetch the caller Term, this is the term whose function is currently being evaluated.
caTerm* circa_caller_term(caStack* stack);

// Fetch the Block that holds the caller Term.
caBlock* circa_caller_block(caStack* stack);

caBlock* circa_top_block(caStack* stack);

// -- Tagged Values --

// Allocate a new caValue container on the heap. This will call circa_init_value for you.
caValue* circa_alloc_value();

// Deallocate a caValue that was created with circa_alloc_value.
void circa_dealloc_value(caValue* value);

// Initialize a newly allocated caValue container. This must be called if you allocate
// the caValue yourself. (such as putting it on the stack).
void circa_init_value(caValue* value);

// Copy a value. Some types may implement this as a lightweight copy, so you will need
// to follow the rules for circa_touch when using the copy.
void circa_copy(caValue* source, caValue* dest);

// Swap values between caValue containers. This is a very cheap operation.
void circa_swap(caValue* left, caValue* right);

// Move a value from 'source' to 'dest'. The existing value at 'dest' will be deallocated,
// and 'source' will contain null.
void circa_move(caValue* source, caValue* dest);

// "Touch" a value, indicating that you are about to start modifying its contents. This
// is only necessary when modifying the elements of a container type (such as a List).
void circa_touch(caValue* value);

// Check two values for equality.
bool circa_equals(caValue* left, caValue* right);

// Allocate a new list value, with the given initial size.
caValue* circa_alloc_list(int size);

caType* circa_type_of(caValue* value);

// Assign a Value using the Type's default create() handler.
void circa_make(caValue* value, caType* type);

// -- Accessors --

// Check the type of a caValue.
bool circa_is_bool(caValue* value);
bool circa_is_block(caValue* value);
bool circa_is_error(caValue* value);
bool circa_is_function(caValue* value);
bool circa_is_float(caValue* value);
bool circa_is_int(caValue* value);
bool circa_is_list(caValue* value);
bool circa_is_null(caValue* value);
bool circa_is_number(caValue* value);
bool circa_is_string(caValue* value);
bool circa_is_type(caValue* value);

// Read the value from a caValue.
bool        circa_bool(caValue* value);
caBlock*    circa_block(caValue* value);
float       circa_float(caValue* value);
caFunction* circa_function(caValue* value);
int         circa_int(caValue* value);
void*       circa_object(caValue* value);
const char* circa_string(caValue* value);
caType*     circa_type(caValue* value);
void        circa_vec2(caValue* vec2, float* xOut, float* yOut);
void        circa_vec3(caValue* vec3, float* xOut, float* yOut, float* zOut);
void        circa_vec4(caValue* vec4, float* xOut, float* yOut, float* zOut, float* wOut);

// Read the pointer value from a caValue. This call will do dereferencing: if the caValue
// is actually a Handle then we'll dereference the handle.
void*       circa_get_pointer(caValue* value);

// Fetch a caValue as a float (converting it from an int if necessary)
float circa_to_float(caValue* value);

// Access an element by index. There are certain rules for using this result:
//   - The element must not be modified, unless you know that you have a writable copy
//     of the container. (Use circa_touch to obtain a writeable copy)
//   - The element pointer becomes invalid when the owning container is resized.
//
// When in doubt, make a copy (circa_copy) of the returned value and use that.
//
caValue* circa_index(caValue* value, int index);

// Number of elements in a list value.
int circa_count(caValue* container);

// -- Writing to a caValue --

// Assign to a caValue.
void circa_set_bool(caValue* container, bool value);
void circa_set_error(caValue* container, const char* msg);
void circa_set_float(caValue* container, float value);
void circa_set_int(caValue* container, int value);
void circa_set_null(caValue* container);
void circa_set_pointer(caValue* container, void* ptr);
void circa_set_term(caValue* container, caTerm* term);
void circa_set_string(caValue* container, const char* str);
void circa_set_typed_pointer(caValue* container, caType* type, void* ptr);
void circa_set_vec2(caValue* container, float x, float y);
void circa_set_vec3(caValue* container, float x, float y, float z);
void circa_set_vec4(caValue* container, float x, float y, float z, float w);

// Assign to a string, with the given length. 'str' does not need to be NULL-terminated.
void circa_set_string_size(caValue* container, const char* str, int size);

// Append to a string
void circa_string_append(caValue* container, const char* str);
void circa_string_append_char(caValue* container, char c);

bool circa_string_equals(caValue* container, const char* str);

// Initialize a list. The container will have length 'numElements' and each element will
// be NULL.
void circa_set_list(caValue* container, int numElements);

// Append a value to a list and return the newly appended value. This return value may be
// modified.
caValue* circa_append(caValue* list);

// Resize a list.
void circa_resize(caValue* list, int count);

// Assign a point.
void circa_set_point(caValue* point, float x, float y);

// Initialize a value with an empty Map.
void circa_set_map(caValue* map);

// Find a value in a Map. The result may not be modified, unless you have a writeable
// copy of the map. Returns NULL if the map doesn't contain the key.
caValue* circa_map_get(caValue* map, caValue* key);

// Insert a key into a map, returning the associated value container. The result value
// may be modified.
caValue* circa_map_insert(caValue* map, caValue* key);

// -- Handle Values --

// Retrive a value from a Handle
caValue* circa_handle_get_value(caValue* handle);

void circa_handle_set_object(caValue* handle, void* object);
void* circa_handle_get_object(caValue* handle);

// -- String Representation --

// Load a Circa value from a string representation. The result will be written to 'out'.
// If there is a parsing error, an error value will be saved to 'out'. (the caller should
// check for this).
void circa_parse_string(const char* str, caValue* out);

// Write a string representation of 'value' to 'out'.
void circa_to_string(caValue* value, caValue* out);

// -- Code Reflection --

// Access the root block for a caWorld.
caBlock* circa_kernel(caWorld* world);

// Find a Term by name, looking in the given block.
caTerm* circa_find_term(caBlock* block, const char* name);

caTerm* circa_find_global(caWorld* world, const char* name);

// Find a Function by name, looking in the given block.
caBlock* circa_find_function_local(caBlock* block, const char* name);

// Find a Type by name, looking in the given block.
caType* circa_find_type_local(caBlock* block, const char* name);

caBlock* circa_find_function(caWorld* world, const char* name);
caType* circa_find_type(caWorld* world, const char* name);

// Retreive the nth input Term to the caller Term. May return NULL if the caller term
// doesn't have that many inputs, or if there is no caller term.
caTerm* circa_caller_input_term(caStack* stack, int index);

// Get a Term from a Block by index.
caTerm* circa_get_term(caBlock* block, int index);

// Get the nth input placeholder from this block. Returns NULL if there is no such input.
caTerm* circa_input_placeholder(caBlock* block, int index);

// Get the nth output placeholder from this block. Returns NULL if there is no such output.
caTerm* circa_output_placeholder(caBlock* block, int index);

// Get the Block contents for a given Term. This may return NULL.
caBlock* circa_nested_block(caTerm* term);

// Get the Block contents for a term with the given name.
caBlock* circa_get_nested_block(caBlock* block, const char* name);

// Get the parent Block for a given Term
caBlock* circa_parent_block(caTerm* term);

// Get the owning Term for a given Block
caTerm* circa_owning_term(caBlock* block);

// Get the Block contents for a function
caBlock* circa_function_contents(caFunction* func);

// Access the fixed value of a value() Term. Returns NULL if Term is not a value.
caValue* circa_term_value(caTerm* term);

// Fetch the term's index (its position inside the parent block)
int circa_term_get_index(caTerm* term);

// Fetch the number of inputs for the given term.
int circa_term_num_inputs(caTerm* term);

// Fetch the nth input for the given term. May return NULL.
caTerm* circa_term_get_input(caTerm* term, int index);

// Fetch the Term's declared type.
caType* circa_term_declared_type(caTerm* term);

// -- Code Building --

// Install an evaluation function to the given named function. Returns the container Term.
// Returns NULL if the name was not found.
caTerm* circa_install_function(caBlock* block, const char* name, caEvaluateFunc func);

typedef struct caFunctionBinding
{
    const char* name;
    caEvaluateFunc func;
} caFunctionBinding;

// Install a series of C function bindings. This will treat 'bindingList' as an array
// that is terminanted with a NULL caFunctionBinding.
void circa_install_function_list(caBlock* block, const caFunctionBinding* bindingList);

// Install an evaluation function to the given Function.
void circa_func_set_evaluate(caFunction* func, caEvaluateFunc evaluate);

// Create a new Function value with the given name. Returns the created Function.
caFunction* circa_declare_function(caBlock* block, const char* name);

// Create a new value() term with the given name. Returns the term's value, which is safe
// to modify.
caValue* circa_declare_value(caBlock* block, const char* name);

// Configure a Circa type so that each value holds an integer.
void circa_setup_int_type(caType* type);

// Configure a Circa type so that each value holds an opaque pointer.
void circa_setup_pointer_type(caType* type);

// -- Native module support --

caNativePatch* circa_create_native_patch(caWorld* world, const char* name);
void circa_patch_function(caNativePatch* module, const char* name, caEvaluateFunc func);
void circa_finish_native_patch(caNativePatch* module);

// -- Actors --
// (Disabled as of actors v3)
#if 0
caActorSpace* circa_create_actor_space(caWorld* world);
caActor* circa_create_actor(caActorSpace* space);
void circa_actor_bind_name(caActorSpace* space, caActor* actor, const char* name);
bool circa_actor_consume_incoming(caActor* actor, caValue* messageOut);
void circa_actors_run_iteration(caActorSpace* space);
caValue* circa_actor_post(caActor* actor);
caValue* circa_post(caActorSpace* space, caValue* address);
#endif

// -- File IO --

void circa_read_file(const char* filename, caValue* contentsOut);
bool circa_file_exists(const char* filename);
int circa_file_get_version(const char* filename);
void circa_get_directory_for_filename(caValue* filename, caValue* result);
void circa_get_parent_directory(caValue* filename, caValue* result);
void circa_chdir(caValue* dir);
void circa_cwd(caValue* cwd);

// -- Builtin REPL --
void circa_repl_start(caStack* stack);
void circa_repl_run_line(caStack* stack, caValue* input, caValue* output);

// -- Debugging Helpers --

// 'dump' commands will print a representation to stdout
void circa_dump_s(caStack* stack);
void circa_dump_b(caBlock* block);

#ifdef __cplusplus
} // extern "C"
#endif

#define CIRCA_EXPORT extern "C"

#endif
