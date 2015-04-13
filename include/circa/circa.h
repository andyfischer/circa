// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#ifndef CIRCA_H_INCLUDED
#define CIRCA_H_INCLUDED

#ifdef _MSC_VER
#else
    #include <cstdbool>
    #include <cstdint>
#endif

// Public API

#ifdef __cplusplus
extern "C" {
#endif

// -- Circa Types --

#ifdef __cplusplus
namespace circa {
    struct Block;
    struct ListData;
    struct NativePatch;
    struct Term;
    struct Type;
    struct World;
    struct Value;
    struct VM;
}

typedef circa::Block caBlock;

typedef circa::VM caVM;

typedef circa::Term caTerm;

// a Type holds data for a single Circa type, including a name, handlers for
// initialization and destruction, and various other handlers and metadata.
typedef circa::Type caType;

// a World holds miscellaneous shared runtime information. Typically, a process should
// create a single World at startup.
typedef circa::World caWorld;

typedef circa::NativePatch caNativePatch;
typedef circa::Value caValue;

#else

typedef struct caBlock caBlock;
typedef struct caVM caVM;
typedef struct caStack caStack;
typedef struct caTerm caTerm;
typedef struct caType caType;
typedef struct caWorld caWorld;
typedef struct caNativePatch caNativePatch;
typedef struct caValue caValue;

#endif

// a Symbol is an alias for an integer, used to reference an interned string.
typedef int caSymbol;

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

#ifdef __cplusplus

namespace circa {

struct Value
{
    caValueData value_data;
    caType* value_type;

    Value();
    ~Value();
    Value(Value const&);
    Value& operator=(Value const&);

    caTerm* asTerm();
    caBlock* asBlock();
    bool asBool();
    caSymbol asSymbol();
    int asInt();
    float asFloat();
    ListData* listData();
    int as_i() { return asInt(); }
    float as_f() { return asFloat(); }
    float to_f();
    caSymbol as_s() { return asSymbol(); }
    const char* as_str();
    caSymbol as_b() { return asBool(); }

    Value* index(int i);
    int length();

    // Assignment
    Value* set(caValue* v) { return set_value(v); }
    Value* set_value(caValue* v);
    Value* set_block(Block* b);
    Value* set_bool(bool b);
    Value* set_string(const char* s);
    Value* set_int(int i);
    Value* set_float(float f);
    Value* set_symbol(caSymbol s);
    Value* set_term(Term* t);

    // List utils
    Value* set_list(int size);
    Value* resize(int size);
    bool isEmpty();
    Value* set_element_int(int index, int i);
    Value* set_element_null(int index);
    Value* set_element_str(int index, const char* s);
    Value* set_element_sym(int index, caSymbol s);
    Value* set_element(int index, Value* val);
    Value* append();
    Value* append_sym(caSymbol s);
    Value* append_str(const char* s);
    Value* extend(Value* rhsList);
    void pop();
    Value* last();

    // Hashtable utils
    Value* set_hashtable();
    Value* field(caSymbol field);
    Value* int_key(int key);
    Value* val_key(Value* key);
    Value* term_key(Term* key);
    Value* block_key(Block* key);
    Value* set_field_int(caSymbol field, int i);
    Value* set_field_str(caSymbol field, const char*);
    Value* set_field_sym(caSymbol field, caSymbol sym);
    Value* set_field_term(caSymbol field, Term* term);
    Value* set_field_bool(caSymbol field, bool b);
    bool field_bool(caSymbol field, bool defaultValue);
    Value* insert(caSymbol key);
    Value* insert_int(int key);
    Value* insert_val(Value* valueKey);
    Value* insert_term(Term* key);

    // For debugging:
    void dump();
    char* to_c_string(); // caller must free() this char*.
};

} // namespace circa

#endif // __cplusplus

// EvaluateFunc is the signature for a C evaluation function. The function will access the
// vm to read inputs (if any), possibly perform some action, and write output values
// (if any) back to the vm.
typedef void (*caEvaluateFunc)(caVM* vm);

typedef void (*caNativePtrRelease)(void* ptr);

typedef void (*caLogFunc)(void* context, const char* msg);

// -- Setting up the Circa environment --

// Create a caWorld. This should be done once at the start of the program.
caWorld* circa_initialize();

// Uninitialize a caWorld. This call is entirely optional, but using it will make memory-
// leak checking tools happier.
void circa_shutdown(caWorld*);

void circa_set_log_handler(caWorld*, void* context, caLogFunc func);

// Add a module search path. This is used when processing 'import' statements.
void circa_add_module_search_path(caWorld* world, const char* path);

// Load a module by opening the given filename as a source file.
caBlock* circa_load_module_from_file(caWorld* world,
                                      const char* module_name,
                                      const char* filename);
caBlock* circa_load_module(caWorld* world, const char* moduleName);

// -- VM --
caVM* circa_new_vm(caWorld* world);
void circa_free_vm(caVM* vm);
void circa_set_main(caVM* vm, caBlock* main);
caValue* circa_vm_get_error(caVM* vm);

// Check for changed files, and run any relevant change actions.
void circa_update_changed_files(caWorld* world);

// Run the VM
void circa_run(caVM* vm);

// Signal that an error has occurred.
void circa_output_error_val(caVM* vm, caValue* val);
void circa_output_error(caVM* vm, const char* msg);

// Return whether a runtime error occurred.
bool circa_has_error(caVM* vm);

void circa_dump_stack_trace(caVM* vm);

// Clear all frames from a Stack.
void circa_clear_stack(caVM* vm);

// Clear all frames but the topmost, rewind the PC, and clear temporary values.
void circa_restart(caVM* vm);

caValue* circa_env_insert(caVM* vm, const char* name);

void circa_throw(caVM* vm, const char* message);

// -- Fetching Inputs & Outputs --

// Get the number of inputs.
int circa_num_inputs(caVM* vm);

// Retrieve the given input value. This value may not be modified.
caValue* circa_input(caVM* vm, int index);

// Read the given input index as an integer.
// Equivalent to: circa_int(circa_input(vm, index))
int circa_int_input(caVM* vm, int index);

// Read the given input index as a string
// Equivalent to: circa_string(circa_input(vm, index))
const char* circa_string_input(caVM* vm, int index);

// Read the given input index as a float
// Equivalent to: circa_to_float(circa_input(vm, index))
// (Note that we use circa_to_float, not circa_float)
float circa_float_input(caVM* vm, int index);

// Read the given input index as a bool
// Equivalent to: circa_bool(circa_input(vm, index))
float circa_bool_input(caVM* vm, int index);

// Fetch the current frame's output value.
caValue* circa_output(caVM* vm);

// Fetch the caller Term, this is the term whose function is currently being evaluated.
caTerm* circa_caller_term(caVM* vm);

// Fetch the Block that holds the caller Term.
caBlock* circa_caller_block(caVM* vm);

caBlock* circa_stack_top_block(caVM* vm);

// -- Tagged Values --
//
// General documentation: Safely using container values.
//
//     When using a container (such as a list or map), you will commonly need to access or
//   modify the contents of the container (such as accessing a list's element). This is
//   done through the use of *deep pointers*. An accessor function, such as circa_index,
//   will return a deep pointer into the list.
//
//     Since Circa uses persistent data structures (two Values may share the same 
//   underlying data), there are rules on safely using deep pointers. The rules are:
//
//   If you are only reading from the deep pointer:
//     - Many operations on the container value will invalidate all deep pointers. It's advised that
//       you don't touch the container at all while accessing the deep pointer.
//     - When in doubt, use circa_copy to copy a deep pointer to a new Value. The newly copied Value
//       is now safe to read from, even if its container is touched.
//
//   If you are also *writing* to a deep pointer:
//     - The above warnings on reading are applicable.
//     - Additionally, you can only write to a deep pointer if its container is deep-write-safe.
//       Check the documentation, each relevant call will describe whether its result is
//       deep-write-safe.
//     - When in doubt, use circa_touch to ensure that a Value is deep-write-safe.
//
//   Terminology
//     - deep-write-safe - A description applied to a container Value, indicating that the
//       value's contents can be safely modified. A newly created value is deep-write-safe,
//       and circa_touch will ensure that a value is deep-write-safe. Sharing a value's data
//       with other values will make that value not deep-write-safe.
//
//     - deep pointer - A (caValue*) that points to the contents of a container. These pointers
//       can easily be made invalid by operations on the owning container.

// Allocate a new caValue container on the heap. This will call circa_init_value for you.
caValue* circa_alloc_value();

// Deallocate a caValue that was created with circa_alloc_value.
void circa_dealloc_value(caValue* value);

// Initialize a newly allocated caValue container to a null value. If you allocated the
// caValue container yourself, then this function must be called before using the value.
// Do not call circa_init_value on an already allocated value, doing so will cause a
// memory leak.
void circa_init_value(caValue* value);

// Create a Value using the type's default initializer.
//   Effect on safety: The result is deep-write-safe.
void circa_make(caValue* value, caType* type);

// Copy a value from 'source' to 'dest'.
//   Effect on safety: 'source' and 'dest' are no longer deep-write-safe.
void circa_copy(caValue* source, caValue* dest);

// Swap values between caValue containers. This is a very cheap operation.
//   Effect on safety: none.
void circa_swap(caValue* left, caValue* right);

// Move a value from 'source' to 'dest', so that no copy takes place. The preexisting value
// at 'dest' is released. After this call, 'source' will be null.
void circa_move(caValue* source, caValue* dest);

// Obtain a deep-write-safe reference to 'value'.
//   Effect on safety: 'value' is now deep-write-safe. Existing deep pointers are invalid.
void circa_touch(caValue* value);

// Check two values for equality.
//   Effect on safety: 'source' and 'dest' are no longer deep-write-safe. Existing deep pointers
//   are invalid.
bool circa_equals(caValue* left, caValue* right);

// Fetch the value's type.
caType* circa_type_of(caValue* value);

// -- Accessors --

// Check the type of a caValue.
bool circa_is_bool(caValue* value);
bool circa_is_blob(caValue* value);
bool circa_is_block(caValue* value);
bool circa_is_error(caValue* value);
bool circa_is_function(caValue* value);
bool circa_is_float(caValue* value);
bool circa_is_int(caValue* value);
bool circa_is_list(caValue* value);
bool circa_is_null(caValue* value);
bool circa_is_number(caValue* value);
bool circa_is_stack(caValue* value);
bool circa_is_string(caValue* value);
bool circa_is_symbol(caValue* value);
bool circa_is_type(caValue* value);

// Read the value from a caValue.
bool        circa_bool(caValue* value);
char*       circa_blob(caValue* value);
caBlock*    circa_block(caValue* value);
float       circa_float(caValue* value);
int         circa_int(caValue* value);
void*       circa_native_ptr(caValue* val);
const char* circa_string(caValue* value);
caType*     circa_type(caValue* value);
void        circa_vec2(caValue* vec2, float* xOut, float* yOut);
void        circa_vec3(caValue* vec3, float* xOut, float* yOut, float* zOut);
void        circa_vec4(caValue* vec4, float* xOut, float* yOut, float* zOut, float* wOut);

void* circa_get_boxed_native_ptr(caValue* val);
void circa_blob_data(caValue* blob, char** dataOut, uint32_t* sizeOut);
uint32_t circa_blob_size(caValue* blob);

// Create a new blob based on a backing value. Caller must guarantee that 'data' is owned
// or contained inside the backingValue.
void circa_set_blob_from_backing_value(caValue* blob, caValue* backingValue, char* data, uint32_t numBytes);

// Read the pointer value from a caValue. This call will do dereferencing: if the caValue
// is actually a Handle then we'll dereference the handle.
// (Deprecated)
void*       circa_get_pointer(caValue* value);

// Fetch a caValue as a float (converting it from an int if necessary)
float circa_to_float(caValue* value);

// Returns the number of elements in a list value.
int circa_length(caValue* container);

// Allocate a new list value, with the given initial size.
caValue* circa_alloc_list(int size);

// Access a list's element by index. The result is a deep pointer. See the above section titled
// "Safely using container values".
caValue* circa_index(caValue* container, int index);

// Access a map's element by key. Returns NULL if the key is not found. The result is a deep
// pointer. See the above section titled "Safely using container values". 
caValue* circa_map_get(caValue* map, caValue* key);

// -- Writing to a caValue --

// Assign to a caValue.
void circa_set_blob(caValue* container, int size);
void circa_set_bool(caValue* container, bool value);
void circa_set_error(caValue* container, const char* msg);
void circa_set_float(caValue* container, float value);
void circa_set_int(caValue* container, int value);
void circa_set_native_ptr(caValue* val, void* ptr, caNativePtrRelease release);
void circa_set_boxed_native_ptr(caValue* val, void* ptr, caNativePtrRelease release);
void circa_set_null(caValue* container);
void circa_set_pointer(caValue* container, void* ptr);
void circa_set_term(caValue* container, caTerm* term);
void circa_set_string(caValue* container, const char* str);
void circa_set_symbol(caValue* container, const char* str);
void circa_set_typed_pointer(caValue* container, caType* type, void* ptr);
void circa_set_vec2(caValue* container, float x, float y);
void circa_set_vec3(caValue* container, float x, float y, float z);
void circa_set_vec4(caValue* container, float x, float y, float z, float w);

// Assign to a string, with the given length. 'str' does not need to be NULL-terminated.
void circa_set_string_size(caValue* container, const char* str, int size);
void circa_set_empty_string(caValue* container, int size);

int circa_string_length(caValue* string);

const char* circa_symbol_text(caValue* symbol);
bool circa_symbol_equals(caValue* symbol, const char* text);

// Append to a string
void circa_string_append(caValue* str, const char* suffix);
void circa_string_append_val(caValue* str, caValue* suffix);
void circa_string_append_len(caValue* str, const char* suffix, int len);
void circa_string_append_char(caValue* str, char suffix);

bool circa_string_equals(caValue* value, const char* str);

// Initialize a list. The container will have length 'numElements' and each element will
// be NULL.
void circa_set_list(caValue* container, int numElements);

// Append a value to a list and return the newly appended value. The result is a deep pointer.
// See the above section titled "Safely using container values".
//   Effect on safety: 'list' is now deep-write-safe, and existing deep pointers are invalid.
caValue* circa_append(caValue* list);

// Resize a list.
//   Effect on safety: Existing deep pointers are invalid.
void circa_resize(caValue* list, int count);

// Initialize a value with an empty Map.
void circa_set_map(caValue* map);

// Access a map's element by key. If the map doesn't contain the key, it is inserted.
// The result is a deep pointer. See the above section titled "Safely using container values".
//   Effect on safety: Existing deep pointers are invalid.
caValue* circa_map_insert(caValue* map, caValue* key);

// Like circa_map_insert, but the 'key' value is moved instead of copied. (see circa_move)
caValue* circa_map_insert_move(caValue* map, caValue* key);

void circa_map_set_int(caValue* map, caValue* key, int val);

void* circa_raw_pointer(caValue* value);
void circa_set_raw_pointer(caValue* value, void* ptr);

// -- String Representation --

// Load a Circa value from a string representation. The result will be written to 'out'.
// If there is a parsing error, an error value will be saved to 'out'. (the caller should
// check for this).
void circa_parse_string(const char* str, caValue* out);
void circa_parse_string_len(const char* str, int len, caValue* out);

// Write a string representation of 'value' to 'out'.
void circa_to_string(caValue* value, caValue* out);

// -- Code Reflection --

// Find a Term by name, looking in the given block.
caTerm* circa_find_term(caBlock* block, const char* name);

// Find a Function by name, looking in the given block.
caBlock* circa_find_function_local(caBlock* block, const char* name);

// Find a Type by name, looking in the given block.
caType* circa_find_type(caBlock* block, const char* name);

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

caBlock* circa_term_get_function(caTerm* term);
caValue* circa_function_get_name(caBlock* func);

// -- Code Building --

// Create a new value() term with the given name. Returns the term's value. The value
// is deep-write-safe.
caValue* circa_declare_value(caBlock* block, const char* name);

// -- Native module support --
caNativePatch* circa_create_native_patch(caWorld* world, const char* name);
void circa_patch_function(caNativePatch* patch, const char* nameStr, caEvaluateFunc func);
void circa_finish_native_patch(caNativePatch* module);

// -- File IO --
void circa_read_file(caWorld* world, const char* filename, caValue* contentsOut);
void circa_read_file_with_vm(caVM* vm, const char* filename, caValue* contentsOut);
bool circa_file_exists(caWorld* world, const char* filename);
int circa_file_get_version(caWorld* world, const char* filename);
void circa_use_local_filesystem(caWorld* world, const char* rootDir);
void circa_use_tarball_filesystem(caWorld* world, caValue* tarball);
void circa_use_in_memory_filesystem(caWorld* world);
void circa_load_file_in_memory(caWorld* world, caValue* filename, caValue* contents);
void circa_load_tar_in_memory(caWorld* world, char* data, uint32_t numBytes);

// -- Path tools --
void circa_get_directory_for_filename(caValue* filename, caValue* result);
void circa_get_parent_directory(caValue* filename, caValue* result);

// Deprecated:
void circa_chdir(caValue* dir);
void circa_cwd(caValue* cwd);

// -- Builtin REPL --
void circa_repl_start(caVM* vm);
void circa_repl_run_line(caVM* vm, caValue* input, caValue* output);

void circa_resolve_possible_module_path(caWorld* world, caValue* dir, caValue* moduleName, caValue* result);
caBlock* circa_load_module_by_filename(caWorld* world, const char* filename);

// -- Debugging --

// 'dump' commands will print a representation to stdout
void circa_dump_s(caVM* vm);
void circa_dump_b(caBlock* block);

void circa_perf_stats_dump();
void circa_perf_stats_reset();

#ifdef __cplusplus
} // extern "C"
#endif

#define CIRCA_EXPORT extern "C"

#endif
