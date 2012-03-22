// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#ifndef CIRCA_H_INCLUDED
#define CIRCA_H_INCLUDED

// Public API

#ifdef __cplusplus
extern "C" {
#endif

// -- Circa Types --

// a Name is an interned string that is referenced by integer.
typedef int caName;

// a Stack holds the interpreter's current state, including a list of activation records.
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
// destroyed.
typedef void (*caReleaseFunc)(caValue* value);

// Initialize Circa. This should be called at startup, before any other Circa functions.
void circa_initialize();

void circa_use_standard_filesystem();

// Shutdown Circa, this should be called before quitting. This isn't required, but calling
// it will make memory-leak checking tools happier.
void circa_shutdown();

// Add a module search path, used when processing 'import' statements.
void circa_add_module_search_path(const char* path);

// Execute with the given command-line args.
int circa_run_command_line(int argc, const char* args[]);

// Load a module by opening the given filename as a source file.
caBranch* circa_load_module_from_file(caName module_name, const char* filename);

// Convert a string to a Circa name (an interned string that is referenced by integer).
// If the name doesn't already exist, it will be created.
caName circa_name(const char* str);

// Retrieve the string for a name.
const char* circa_name_string(caName name);

// -- Controlling the Interpreter --

// Allocate a new Stack object.
caStack* circa_alloc_stack();

// Deallocate a Stack object.
void circa_dealloc_stack(caStack* stack);

// Execute the named module.
void circa_run_module(caStack* stack, caName moduleName);

// Return whether a runtime error occurred.
bool circa_has_error(caStack* stack);

// Clear a runtime error from the stack.
void circa_clear_error(caStack* stack);

// Print a human-readable description of the stack's error to stdout.
void circa_print_error_to_stdout(caStack* stack);

// -- Fetching Inputs & Outputs --

// Retrieve the given input value. This value may not be modified.
caValue* circa_input(caStack* stack, int index);

// Read the given input index as an integer
int circa_int_input(caStack* stack, int index);

// Read the given input index as a string
const char* circa_string_input(caStack* stack, int index);

// Fetch the caValue slot for the given output index.
caValue* circa_output(caStack* stack, int index);

// Initialize the given output value using the default create() function on the
// Term's declared type. Also returns the output slot.
caValue* circa_create_default_output(caStack* stack, int index);

// Fetch the Term that is currently being evaluated.
caTerm* circa_current_term(caStack* stack);

// Fetch the Branch for the function that is currently being evaluated
caBranch* circa_callee_branch(caStack* stack);

// -- Working With Tagged Values --

// Allocate a new caValue container on the heap. This will call circa_init_value for you.
caValue* circa_alloc_value();

// Deallocate a caValue that was created with circa_alloc_value.
void circa_dealloc_value(caValue* value);

// Initialize a newly allocated caValue container. This must be called before any set()
// functions.
void circa_init_value(caValue* container);

bool circa_is_int(caValue* container);
bool circa_is_float(caValue* container);
bool circa_is_string(caValue* container);

// Read an integer from a caValue
int circa_as_int(caValue* container);
float circa_as_float(caValue* container);

// Read a caValue as a string
const char* circa_as_string(caValue* container);

// Convert this caValue to a new string. If the value is already string, then this
// returns an exact copy (no quote marks are added). Otherwise it uses the type's
// to string handler. Either way, the result is a newly allocated string and the
// caller must free() it when finished.
char* circa_to_string(caValue* value);

void circa_get_point(caValue* point, float* xOut, float* yOut);
void circa_get_vec3(caValue* vec3, float* xOut, float* yOut, float* zOut);
void circa_get_vec4(caValue* vec4, float* xOut, float* yOut, float* zOut, float* wOut);
void circa_get_color(caValue* color, float* rOut, float* gOut, float* bOut, float* aOut);

// Read an opaque pointer from a caValue
void* circa_as_pointer(caValue* container);

// Assign a null value
void circa_set_null(caValue* container);

// Assign an integer
void circa_set_int(caValue* container, int value);

// Assign a float
void circa_set_float(caValue* container, float value);

// Assign a boolean
void circa_set_bool(caValue* container, bool value);

// Assign an opaque pointer
void circa_set_pointer(caValue* container, void* ptr);

// Assign a string
void circa_set_string(caValue* container, const char* str);

// Assign a string with the given length ('str' does not need to be null-terminated)
void circa_set_string_size(caValue* container, const char* str, int size);

// Append a value to a string
void circa_string_append(caValue* container, const char* str);

// Assign a list. The container will have length 'numElements' and each element will be NULL
void circa_set_list(caValue* container, int numElements);

// Append a value to a list and return it. The returned Value may be modified.
caValue* circa_list_append(caValue* list);

// Assign a point.
void circa_set_point(caValue* point, float x, float y);

// -- Handle Values --

// Retrive a value from a Handle
caValue* circa_handle_get_value(caValue* handle);

// Assign a value to a Handle
void circa_handle_set(caValue* container, caValue* value, caReleaseFunc releaseFunc);

void circa_handle_set_object(caValue* handle, void* object, caReleaseFunc releaseFunc);

void* circa_handle_get_object(caValue* handle);

// Assign a Value using the Type's default create() handler.
void circa_create_value(caValue* value, caType* type);

// Signal that an error has occurred
void circa_raise_error(caStack* stack, const char* msg);

// -- String Representation --

// Load a Circa value from a string representation. The result will be written to 'out'.
// If there is a parsing error, an error value will be saved to 'out'. (the caller should
// check for this).
void circa_parse_string(const char* str, caValue* out);

// Write a string representation of 'value' to 'out'.
void circa_to_string_repr(caValue* value, caValue* out);

// -- Code Reflection --

// Retrive the nth input Term for the stack's current Term. May return NULL.
// This is equivalent to: circa_term_get_input(circa_current_term(stack), index)
caTerm* circa_input_term(caStack* stack, int index);

// Get a Term from a Branch by index.
caTerm* circa_get_term(caBranch* branch, int index);

// Get the Branch contents for a given Term. This may return NULL.
caBranch* circa_nested_branch(caTerm* term);

// Get the Branch contents for a term with the given name.
caBranch* circa_get_nested_branch(caBranch* branch, const char* name);

// Get the Branch contents for a function
caBranch* circa_function_contents(caFunction* func);

// Access the fixed value of a value() Term. Returns NULL if Term is not a value.
caValue* circa_term_value(caTerm* term);

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
caTerm* circa_install_function(caBranch* branch, const char* name, caEvaluateFunc evaluate);

// Install an evaluation function to the given Function.
void circa_func_set_evaluate(caFunction* func, caEvaluateFunc evaluate);

// Create a new Function value with the given name. Returns the created Function.
caFunction* circa_declare_function(caBranch* branch, const char* name);

// Create a new value() term with the given name. Returns the term's value, which is safe
// to modify.
caValue* circa_declare_value(caBranch* branch, const char* name);

// -- Debugging Helpers --

// 'dump' commands will print a representation to stdout
void circa_dump_s(caStack* stack);
void circa_dump_b(caBranch* branch);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
