// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#ifndef CIRCA_H_INCLUDED
#define CIRCA_H_INCLUDED

#include <stdbool.h>

// Public API

#ifdef __cplusplus
extern "C" {
#endif

// -- Circa Types --

// a Name is an interned string that is referenced by integer.
typedef int caName;

// a Stack holds the interpreter's current state, including a list of frames (activation records)
typedef struct caStack caStack;

// a Value is a variant value. It holds two pointers, one pointer to the Type object and one
// to the value's data. For some types (such as integers, floats, booleans), the data section
// holds the actual value and not a pointer.
typedef struct caValue caValue;

// a Branch is a section of compiled code. It contains a list of Terms and some other metadata.
// Each Term may itself contain a nested Branch.
typedef struct caBranch caBranch;

// a Term is one unit of compiled code. Each term has a function and a list of inputs. A term
// may also have a nested Branch.
typedef struct caTerm caTerm;

// a Type holds data for a single Circa type, including a name, handlers for initialization
// and destruction, and various other handlers and metadata.
typedef struct caType caType;

// a Function holds data for a single Circa function, including a name, the function's definition
// (stored as a Branch), and various other metadata. Each Function has an EvaluateFunc which
// is triggered when the function is called.
typedef struct caFunction caFunction;

// EvaluateFunc is the signature for a C evaluation function. The function will access the stack
// to read inputs (if any), possibly perform some action, and write output values (if any) back
// to the stack.
typedef void (*caEvaluateFunc)(caStack* stack);

// ReleaseFunc is the signature for a C function that runs when a user object is about to be
// destroyed. This is used for 'handle' values.
typedef void (*caReleaseFunc)(caValue* value);

// -- Setting up the Circa environment --

// Initialize Circa. This should be called at startup, before any other Circa functions.
void circ_initialize();

// Shutdown Circa. This should be called before quitting. This isn't required, but using
// it will make memory-leak detection tools happier.
void circ_shutdown();

// Set up a POSIX file handler as a file source. For more control over file sources, see
// circa/file.h
void circ_use_standard_filesystem();

// Add a module search path. This is used when processing 'import' statements.
void circ_add_module_search_path(const char* path);

// Execute the command-line handler with the given arguments.
int circ_run_command_line(int argc, const char* args[]);

// Load a module by opening the given filename as a source file.
caBranch* circ_load_module_from_file(caName module_name, const char* filename);

// Convert a string to a Circa name (an interned string that is referenced by integer).
// If the name doesn't already exist, it will be created.
caName circ_name(const char* str);

// Retrieve the string for a name.
const char* circ_name_string(caName name);

// -- Controlling the Interpreter --

// Allocate a new Stack object.
caStack* circ_alloc_stack();

// Deallocate a Stack object.
void circ_dealloc_stack(caStack* stack);

// Execute the named module.
void circ_run_module(caStack* stack, caName moduleName);

// Execute the given function with the given inputs. 'Inputs' must either be a list,
// or NULL if there are no inputs.
void circ_run_function(caStack* stack, caFunction* function, caValue* inputs);

// Signal that an error has occurred.
void circ_raise_error(caStack* stack, const char* msg);

// Return whether a runtime error occurred.
bool circ_has_error(caStack* stack);

// Clear a runtime error from the stack.
void circ_clear_error(caStack* stack);

// Print a human-readable description of the stack's error to stdout.
void circ_print_error_to_stdout(caStack* stack);

// -- Fetching Inputs & Outputs --

// Retrieve the given input value. This value may not be modified.
caValue* circ_input(caStack* stack, int index);

// Read the given input index as an integer.
// Equivalent to: circ_int(circ_input(stack, index))
int circ_int_input(caStack* stack, int index);

// Read the given input index as a string
// Equivalent to: circ_string(circ_input(stack, index))
const char* circ_string_input(caStack* stack, int index);

// Read the given input index as a float
// Equivalent to: circ_float(circ_input(stack, index))
float circ_float_input(caStack* stack, int index);

// Fetch the given output value. This value should be modified.
caValue* circ_output(caStack* stack, int index);

// Initialize the given output value, using the default create() function on the
// Term's declared type. Also, returns the output value for convenient writing.
// This function is unnecessary if you write to the output using one of the
// circ_set_XXX functions.
caValue* circ_create_default_output(caStack* stack, int index);

// Fetch the Term that is currently being evaluated.
caTerm* circ_current_term(caStack* stack);

// Fetch the Branch for the function that is currently being evaluated
caBranch* circ_callee_branch(caStack* stack);

// -- Tagged Values --

// Allocate a new caValue container on the heap. This will call circ_init_value for you.
caValue* circ_alloc_value();

// Deallocate a caValue that was created with circ_alloc_value.
void circ_dealloc_value(caValue* value);

// Initialize a newly allocated caValue container. This must be called if you allocate
// the caValue yourself. (such as putting it on the stack).
void circ_init_value(caValue* value);

// -- Accessors --

// Check the type of a caValue.
bool circ_is_bool(caValue* value);
bool circ_is_branch(caValue* value);
bool circ_is_error(caValue* value);
bool circ_is_function(caValue* value);
bool circ_is_float(caValue* value);
bool circ_is_int(caValue* value);
bool circ_is_list(caValue* value);
bool circ_is_name(caValue* value);
bool circ_is_null(caValue* value);
bool circ_is_number(caValue* value);
bool circ_is_string(caValue* value);
bool circ_is_type(caValue* value);

// Read the value from a caValue.
bool        circ_bool(caValue* value);
caBranch*   circ_branch(caValue* value);
float       circ_float(caValue* value);
caFunction* circ_function(caValue* value);
int         circ_int(caValue* value);
caName      circ_name(caValue* value);
const char* circ_string(caValue* value);
caType*     circ_type(caValue* value);
void        circ_vec2(caValue* vec2, float* xOut, float* yOut);
void        circ_vec3(caValue* vec3, float* xOut, float* yOut, float* zOut);
void        circ_vec4(caValue* vec4, float* xOut, float* yOut, float* zOut, float* wOut);

// Read the pointer value from a caValue. This call will do dereferencing: if the caValue
// is actually a Handle then we'll dereference the handle.
void*       circ_get_pointer(caValue* value);

// Fetch a caValue as a float (converting it from an int if necessary)
float circ_to_float(caValue* value);

// Access an element by index. There are certain rules when accessing a container's element:
//   - The element must not be modified, unless you know that you have a writable copy
//     of the container. To obtain a writable copy, first call circ_touch on the container.
//   - The element pointer must be considered invalid when the owning container is resized.
caValue* circ_index(caValue* value, int index);

// Number of elements in a list value.
int circ_count(caValue* container);

// -- Writing to a caValue --

// "Touch" a value, indicating that you are about to start modifying its contents. This
// is only necessary for a container type (such as a List), and it's not necessary if you
// are the only owner of this value.
void circ_touch(caValue* value);

// Assign to a caValue.
void circ_set_bool(caValue* container, bool value);
void circ_set_float(caValue* container, float value);
void circ_set_int(caValue* container, int value);
void circ_set_null(caValue* container);
void circ_set_pointer(caValue* container, void* ptr);
void circ_set_term(caValue* container, caTerm* term);
void circ_set_string(caValue* container, const char* str);
void circ_set_typed_pointer(caValue* container, caType* type, void* ptr);
void circ_set_vec2(caValue* container, float x, float y);
void circ_set_vec3(caValue* container, float x, float y, float z);
void circ_set_vec4(caValue* container, float x, float y, float z, float w);

// Assign to a string, with the given length. 'str' does not need to be NULL-terminated
// here.
void circ_set_string_size(caValue* container, const char* str, int size);

// Append a value to a string
void circ_string_append(caValue* container, const char* str);

// Initialize a list. The container will have length 'numElements' and each element will
// be NULL.
void circ_set_list(caValue* container, int numElements);

// Append a value to a list and return the newly appended value. This return value may be
// modified.
caValue* circ_append(caValue* list);

// Resize a list.
void circ_resize(caValue* list, int count);

// Assign a point.
void circ_set_point(caValue* point, float x, float y);

// -- Handle Values --

// Retrive a value from a Handle
caValue* circ_handle_get_value(caValue* handle);

// Assign a value to a Handle
void circ_handle_set(caValue* container, caValue* value, caReleaseFunc releaseFunc);

void circ_handle_set_object(caValue* handle, void* object, caReleaseFunc releaseFunc);

void* circ_handle_get_object(caValue* handle);

// Assign a Value using the Type's default create() handler.
void circ_create_value(caValue* value, caType* type);

// -- String Representation --

// Load a Circa value from a string representation. The result will be written to 'out'.
// If there is a parsing error, an error value will be saved to 'out'. (the caller should
// check for this).
void circ_parse_string(const char* str, caValue* out);

// Write a string representation of 'value' to 'out'.
void circ_to_string_repr(caValue* value, caValue* out);

// -- Code Reflection --

// Find a Term by name, looking in the given branch.
caTerm* circ_find_term(caBranch* branch, const char* name);

// Find a Function by name, looking in the given branch.
caFunction* circ_find_function(caBranch* branch, const char* name);

// Find a Type by name, looking in the given branch.
caType* circ_find_type(caBranch* branch, const char* name);

// Retrive the nth input Term for the stack's current Term. May return NULL.
// This is equivalent to: circ_term_get_input(circ_current_term(stack), index)
caTerm* circ_input_term(caStack* stack, int index);

// Get a Term from a Branch by index.
caTerm* circ_get_term(caBranch* branch, int index);

// Get the Branch contents for a given Term. This may return NULL.
caBranch* circ_nested_branch(caTerm* term);

// Get the Branch contents for a term with the given name.
caBranch* circ_get_nested_branch(caBranch* branch, const char* name);

// Get the Branch contents for a function
caBranch* circ_function_contents(caFunction* func);

// Access the fixed value of a value() Term. Returns NULL if Term is not a value.
caValue* circ_term_value(caTerm* term);

int circ_term_get_index(caTerm* term);

// Fetch the number of inputs for the given term.
int circ_term_num_inputs(caTerm* term);

// Fetch the nth input for the given term. May return NULL.
caTerm* circ_term_get_input(caTerm* term, int index);

// Fetch the Term's declared type.
caType* circ_term_declared_type(caTerm* term);

// -- Code Building --

// Install an evaluation function to the given named function. Returns the container Term.
// Returns NULL if the name was not found.
caTerm* circ_install_function(caBranch* branch, const char* name, caEvaluateFunc evaluate);

typedef struct caFunctionBinding
{
    const char* name;
    caEvaluateFunc func;
} caFunctionBinding;

// Install a series of C function bindings. This will treat 'bindingList' as an array
// that is terminanted with a NULL caFunctionBinding.
void circ_install_function_list(caBranch* branch, const caFunctionBinding* bindingList);

// Install an evaluation function to the given Function.
void circ_func_set_evaluate(caFunction* func, caEvaluateFunc evaluate);

// Create a new Function value with the given name. Returns the created Function.
caFunction* circ_declare_function(caBranch* branch, const char* name);

// Create a new value() term with the given name. Returns the term's value, which is safe
// to modify.
caValue* circ_declare_value(caBranch* branch, const char* name);

// -- Debugging Helpers --

// 'dump' commands will print a representation to stdout
void circ_dump_s(caStack* stack);
void circ_dump_b(caBranch* branch);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
