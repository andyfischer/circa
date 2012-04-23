// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa/circa.h"
#include "circa/file.h"

#include "branch.h"
#include "building.h"
#include "code_iterators.h"
#include "dict.h"
#include "evaluation.h"
#include "filesystem.h"
#include "function.h"
#include "gc.h"
#include "generic.h"
#include "importing.h"
#include "introspection.h"
#include "kernel.h"
#include "list.h"
#include "modules.h"
#include "parser.h"
#include "subroutine.h"
#include "source_repro.h"
#include "stateful_code.h"
#include "static_checking.h"
#include "string_type.h"
#include "names.h"
#include "term.h"
#include "type.h"

#include "types/any.h"
#include "types/bool.h"
#include "types/callable.h"
#include "types/color.h"
#include "types/common.h"
#include "types/eval_context.h"
#include "types/hashtable.h"
#include "types/indexable.h"
#include "types/int.h"
#include "types/name.h"
#include "types/number.h"
#include "types/ref.h"
#include "types/set.h"
#include "types/void.h"

#include "tools/command_reader.h"

typedef struct caWorld {
    // Opaque type
    // Currently is empty
} caWorld;

namespace circa {

Branch* KERNEL = NULL;

// STDLIB_CA_TEXT is defined in generated/stdlib_script_text.cpp
extern "C" {
    extern const char* STDLIB_CA_TEXT;
}

// setup_functions is defined in generated/setup_builtin_functions.cpp
void setup_builtin_functions(Branch*);

bool STATIC_INITIALIZATION_FINISHED = false;
bool FINISHED_BOOTSTRAP = false;
bool SHUTTING_DOWN = false;

Term* APPLY_FEEDBACK = NULL;
Term* AVERAGE_FUNC = NULL;
Term* DESIRED_VALUE_FEEDBACK = NULL;
Term* DO_ONCE_FUNC = NULL;
Term* ERRORED_FUNC = NULL;
Term* EXTRA_OUTPUT_FUNC = NULL;
Term* FEEDBACK_FUNC = NULL;
Term* FREEZE_FUNC = NULL;
Term* INSTANCE_FUNC = NULL;
Term* LIST_TYPE = NULL;
Term* LIST_APPEND_FUNC = NULL;
Term* NAMESPACE_FUNC = NULL;
Term* OVERLOADED_FUNCTION_FUNC = NULL;
Term* REF_FUNC = NULL;
Term* SWITCH_FUNC = NULL;
Term* STATEFUL_VALUE_FUNC = NULL;
Term* STATIC_ERROR_FUNC = NULL;
Term* UNKNOWN_IDENTIFIER_FUNC = NULL;
Term* UNRECOGNIZED_EXPRESSION_FUNC = NULL;

Term* ANY_TYPE = NULL;
Term* BOOL_TYPE = NULL;
Term* DICT_TYPE = NULL;
Term* FLOAT_TYPE = NULL;
Term* INT_TYPE = NULL;
Term* NULL_T_TERM = NULL;
Term* RECT_I_TYPE_TERM = NULL;
Term* REF_TYPE = NULL;
Term* STRING_TYPE = NULL;
Term* FEEDBACK_TYPE = NULL;
Term* FUNCTION_TYPE = NULL;
Term* MAP_TYPE = NULL;
Term* NAME_TYPE = NULL;
Term* TYPE_TYPE = NULL;
Term* VOID_TYPE = NULL;
Term* OPAQUE_POINTER_TYPE = NULL;

// New style for builtin function pointers
BuiltinFuncs FUNCS;

// Builtin type objects:
Type ANY_T;
Type BOOL_T;
Type BRANCH_T;
Type DICT_T;
Type ERROR_T;
Type EVAL_CONTEXT_T;
Type FLOAT_T;
Type FUNCTION_T;
Type FUNCTION_ATTRS_T;
Type HANDLE_T;
Type INT_T;
Type LIST_T;
Type NULL_T;
Type OPAQUE_POINTER_T;
Type REF_T;
Type STRING_T;
Type NAME_T;
Type TYPE_T;
Type VOID_T;

BuiltinTypes TYPES;

Value TrueValue;
Value FalseValue;

namespace cppbuild_function { void build_module(caStack*); }

Type* output_placeholder_specializeType(Term* caller)
{
    if (caller->input(0) == NULL)
        return NULL;
    return declared_type(caller->input(0));
}

void file__exists(caStack* stack)
{
    set_bool(circa_output(stack, 0), circa_file_exists( circa_string_input(stack, 0)));
}
void file__version(caStack* stack)
{
    set_int(circa_output(stack, 0), circa_file_get_version(circa_string_input(stack, 0)));
}
void file__read_text(caStack* stack)
{
    set_string(circa_output(stack, 0), circa_read_file(circa_string_input(stack, 0)));
}

void file__fetch_record(caStack* stack)
{
    const char* filename = circa_string_input(stack, 0);
    Name name = circa_int_input(stack, 1);
    circa_create_default_output(stack, 0);
    set_pointer(circa_output(stack, 0), circa_fetch_file_record(filename, name));
}

void from_string(caStack* stack)
{
    circa_parse_string(circa_string_input(stack, 0), circa_output(stack, 0));
}

void to_string_repr(caStack* stack)
{
    circa_to_string_repr(circa_input(stack, 0), circa_output(stack, 0));
}

void dynamic_call_func(caStack* stack)
{
    caValue* callable = circa_input(stack, 0);
    Value inputs;
    circa_swap(circa_input(stack, 1), &inputs);

    caBranch* branch = NULL;

    if (circa_is_branch(callable))
        branch = circa_branch(callable);
    else if (circa_is_function(callable))
        branch = circa_function_contents(circa_function(callable));
    else
        branch = (Branch*) circa_nested_branch(circa_caller_input_term(stack, 0));

    if (branch == NULL) {
        circa_output_error(stack, "Input 0 is not callable");
        return;
    }

    // Pop calling frame
    pop_frame((EvalContext*) stack);

    // Replace it with the callee frame
    push_frame_with_inputs((EvalContext*) stack, (Branch*) branch, &inputs);
}

void dynamic_method_call(caStack* stack)
{
    caValue* args = circa_input(stack, 0);

    // Lookup method
    Term* term = (Term*) circa_caller_term(stack);
    caValue* object = circa_index(args, 0);
    std::string functionName = term->stringPropOptional("syntax:functionName", "");

    // Find and dispatch method
    Term* method = find_method((Branch*) top_branch((EvalContext*) stack),
        (Type*) circa_type_of(object), functionName.c_str());

    if (method != NULL) {
        // Grab inputs before pop
        Value inputs;
        swap(args, &inputs);

        pop_frame((EvalContext*) stack);
        push_frame_with_inputs((EvalContext*) stack, function_contents(method), &inputs);
        return;
    }

    // No method found. Fall back to a field access. This is deprecated behavior.
    if (is_list_based_type(object->value_type)) {
        int fieldIndex = list_find_field_index_by_name(object->value_type, functionName.c_str());
        if (fieldIndex == -1) {
            set_null(circa_output(stack, 0));
            return;
        }
        caValue* element = get_index(object, fieldIndex);
        if (element == NULL)
            set_null(circa_output(stack, 0));
        else
            copy(element, circa_output(stack, 0));
    }

}

void refactor__rename(caStack* stack)
{
    rename(as_ref(circa_input(stack, 0)), as_string(circa_input(stack, 1)));
}

void refactor__change_function(caStack* stack)
{
    change_function(as_ref(circa_input(stack, 0)),
        (Term*) circa_caller_input_term(stack, 1));
}

void reflect__this_branch(caStack* stack)
{
    set_branch(circa_output(stack, 0), (Branch*) circa_caller_branch(stack));
}

void reflect__kernel(caStack* stack)
{
    set_branch(circa_output(stack, 0), kernel());
}

void sys__module_search_paths(caStack* stack)
{
    copy(modules_get_search_paths(), circa_output(stack, 0));
}

void sys__do_admin_command(caStack* stack)
{
    do_admin_command(circa_input(stack, 0), circa_output(stack, 0));
}

void branch_ref(caStack* stack)
{
    Term* input0 = (Term*) circa_caller_input_term(stack, 0);
    Branch* branch = input0->nestedContents;
    gc_mark_object_referenced(&branch->header);
    set_branch(circa_output(stack, 0), branch);
}

void load_script_value(caStack* stack)
{
    const char* filename = circa_string_input(stack, 0);
    
    Branch* branch = alloc_branch_gc();
    load_script(branch, filename);

    set_branch(circa_output(stack, 0), branch);
}

void Branch__dump(caStack* stack)
{
    dump(as_branch(circa_input(stack, 0)));
}

void Branch__input(caStack* stack)
{
    Branch* branch = as_branch(circa_input(stack, 0));
    set_ref(circa_output(stack, 0),
        get_input_placeholder(branch, circa_int_input(stack, 1)));
}
void Branch__inputs(caStack* stack)
{
    Branch* branch = as_branch(circa_input(stack, 0));
    caValue* output = circa_output(stack, 0);
    set_list(output, 0);
    for (int i=0;; i++) {
        Term* term = get_input_placeholder(branch, i);
        if (term == NULL)
            break;
        set_ref(list_append(output), term);
    }
}
void Branch__output(caStack* stack)
{
    Branch* branch = as_branch(circa_input(stack, 0));
    set_ref(circa_output(stack, 0),
        get_output_placeholder(branch, circa_int_input(stack, 1)));
}
void Branch__outputs(caStack* stack)
{
    Branch* branch = as_branch(circa_input(stack, 0));
    caValue* output = circa_output(stack, 0);
    set_list(output, 0);
    for (int i=0;; i++) {
        Term* term = get_output_placeholder(branch, i);
        if (term == NULL)
            break;
        set_ref(list_append(output), term);
    }
}

void Branch__format_source(caStack* stack)
{
    Branch* branch = as_branch(circa_input(stack, 0));

    caValue* output = circa_output(stack, 0);
    circa_set_list(output, 0);
    format_branch_source((StyledSource*) output, branch);
}

void Branch__has_static_error(caStack* stack)
{
    Branch* branch = as_branch(circa_input(stack, 0));
    set_bool(circa_output(stack, 0), has_static_errors_cached(branch));
}

void Branch__get_static_errors(caStack* stack)
{
    Branch* branch = as_branch(circa_input(stack, 0));

    if (is_null(&branch->staticErrors))
        set_list(circa_output(stack, 0), 0);
    else
        copy(&branch->staticErrors, circa_output(stack, 0));
}

void Branch__get_static_errors_formatted(caStack* stack)
{
    Branch* branch = as_branch(circa_input(stack, 0));

    if (is_null(&branch->staticErrors))
        set_list(circa_output(stack, 0), 0);

    caValue* errors = &branch->staticErrors;
    caValue* out = circa_output(stack, 0);
    set_list(out, circa_count(errors));
    for (int i=0; i < circa_count(out); i++)
        format_static_error(circa_index(errors, i), circa_index(out, i));
}

void Branch__evaluate(caStack* stack)
{
    Branch* branch = as_branch(circa_input(stack, 0));
    push_frame((EvalContext*) stack, branch);
}

// Reflection

void Branch__terms(caStack* stack)
{
    Branch* branch = as_branch(circa_input(stack, 0));

    caValue* out = circa_output(stack, 0);
    set_list(out, branch->length());

    for (int i=0; i < branch->length(); i++)
        set_ref(circa_index(out, i), branch->get(i));
}

void Branch__get_term(caStack* stack)
{
    Branch* branch = as_branch(circa_input(stack, 0));

    int index = circa_int_input(stack, 1);
    set_ref(circa_output(stack, 0), branch->get(index));
}

bool is_considered_config(Term* term)
{
    if (term == NULL) return false;
    if (term->name == "") return false;
    if (!is_value(term)) return false;
    if (is_declared_state(term)) return false;
    if (is_hidden(term)) return false;
    if (is_function(term)) return false;

    // ignore branch-based types
    //if (is_branch(term)) return false;
    if (is_type(term)) return false;

    return true;
}

void Branch__list_configs(caStack* stack)
{
    Branch* branch = as_branch(circa_input(stack, 0));

    caValue* output = circa_output(stack, 0);

    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);
        if (is_considered_config(term))
            set_ref(circa_append(output), term);
    }
}

void Branch__functions(caStack* stack)
{
    Branch* branch = as_branch(circa_input(stack, 0));

    caValue* output = circa_output(stack, 0);
    set_list(output, 0);

    for (BranchIteratorFlat it(branch); it.unfinished(); it.advance()) {
        Term* term = *it;
        if (is_function(term)) {
            set_function(list_append(output), as_function(term));
        }
    }
}

void Branch__file_signature(caStack* stack)
{
    Branch* branch = as_branch(circa_input(stack, 0));
    List* fileOrigin = branch_get_file_origin(branch);
    if (fileOrigin == NULL)
        set_null(circa_output(stack, 0));
    else
    {
        List* output = set_list(circa_output(stack, 0), 2);
        copy(fileOrigin->get(1), output->get(0));
        copy(fileOrigin->get(2), output->get(1));
    }
}

void Branch__statements(caStack* stack)
{
    Branch* branch = (Branch*) circa_branch(circa_input(stack, 0));

    caValue* out = circa_output(stack, 0);

    circa_set_list(out, 0);

    for (int i=0; i < branch->length(); i++)
        if (is_statement(branch->get(i)))
            circa_set_term(circa_append(out), (caTerm*) branch->get(i));
}

void Frame__branch(caStack* stack)
{
}

void Frame__register(caStack* stack)
{
}

void Frame__pc(caStack* stack)
{
}

void Function__name(caStack* stack)
{
    set_string(circa_output(stack, 0), as_function(circa_input(stack, 0))->name);
}

void Function__input(caStack* stack)
{
    int index = circa_int_input(stack, 1);
    set_ref(circa_output(stack, 0), function_get_input_placeholder(as_function(circa_input(stack, 0)), index));
}

void Function__inputs(caStack* stack)
{
    Function* func = as_function(circa_input(stack, 0));
    caValue* output = circa_output(stack, 0);
    set_list(output, 0);
    for (int i=0;; i++) {
        Term* term = function_get_input_placeholder(func, i);
        if (term == NULL)
            break;
        set_ref(list_append(output), term);
    }
}

void Function__output(caStack* stack)
{
    int index = circa_int_input(stack, 1);
    set_ref(circa_output(stack, 0), function_get_output_placeholder(as_function(circa_input(stack, 0)), index));
}

void Function__outputs(caStack* stack)
{
    Function* func = as_function(circa_input(stack, 0));
    caValue* output = circa_output(stack, 0);
    set_list(output, 0);
    for (int i=0;; i++) {
        Term* term = function_get_output_placeholder(func, i);
        if (term == NULL)
            break;
        set_ref(list_append(output), term);
    }
}

void Function__contents(caStack* stack)
{
    set_branch(circa_output(stack, 0), function_get_contents(as_function(circa_input(stack, 0))));
}

void make_interpreter(caStack* stack)
{
}

void Interpreter__push_frame(caStack* stack)
{
}
void Interpreter__run_step(caStack* stack)
{
}
void Interpreter__run_steps(caStack* stack)
{
}
void Interpreter__frame(caStack* stack)
{
}

void String__char_at(caStack* stack)
{
    const char* str = circa_string_input(stack, 0);
    int index = circa_int_input(stack, 1);

    if (index < 0) {
        circa_output_error(stack, "negative index");
        return;
    }

    if ((unsigned) index >= strlen(str)) {
        set_string(circa_output(stack, 0), "");
        return;
    }

    char output[1];
    output[0] = str[index];
    set_string(circa_output(stack, 0), output, 1);
}

void String__length(caStack* stack)
{
    set_int(circa_output(stack, 0), int(circa_input(stack, 0)->asString().length()));
}

void String__substr(caStack* stack)
{
    int start = circa_int_input(stack, 1);
    int end = circa_int_input(stack, 2);
    std::string const& s = as_string(circa_input(stack, 0));

    if (start < 0) return circa_output_error(stack, "Negative index");
    if (end < 0) return circa_output_error(stack, "Negative index");

    if ((unsigned) start > s.length()) {
        std::stringstream msg;
        msg << "Start index is too high: " << start;
        return circa_output_error(stack, msg.str().c_str());
    }
    if ((unsigned) (start+end) > s.length()) {
        std::stringstream msg;
        msg << "End index is too high: " << start;
        return circa_output_error(stack, msg.str().c_str());
    }

    set_string(circa_output(stack, 0), s.substr(start, end));
}

char character_to_lower(char c)
{
    if (c >= 'A' && c <= 'Z')
        return c + 'a' - 'A';
    return c;
}

void String__to_camel_case(caStack* stack)
{
    const char* in = circa_string_input(stack, 0);
    set_string(circa_output(stack, 0), in);

    char* out = (char*) as_cstring(circa_output(stack, 0));
    if (out[0] == 0)
        return;

    out[0] = character_to_lower(out[0]);
}

void String__to_lower(caStack* stack)
{
    const char* in = circa_string_input(stack, 0);
    int len = strlen(in);

    set_string(circa_output(stack, 0), in);
    char* out = (char*) as_cstring(circa_output(stack, 0));

    for (int i=0; i < len; i++) {
        char c = in[i];

        if (c >= 'A' && c <= 'Z')
            c = c + 'a' - 'A';
        out[i] = c;
    }
}

void String__to_upper(caStack* stack)
{
    const char* in = circa_string_input(stack, 0);
    int len = strlen(in);

    set_string(circa_output(stack, 0), in);
    char* out = (char*) as_cstring(circa_output(stack, 0));

    for (int i=0; i < len; i++) {
        char c = in[i];

        if (c >= 'a' && c <= 'z')
            c = c + 'A' - 'a';
        out[i] = c;
    }
}

void String__slice(caStack* stack)
{
    int start = circa_int_input(stack, 1);
    int end = circa_int_input(stack, 2);
    std::string const& s = as_string(circa_input(stack, 0));

    // Negative indexes are relatve to end of string
    if (start < 0) start = s.length() + start;
    if (end < 0) end = s.length() + end;

    if (start < 0) return set_string(circa_output(stack, 0), "");
    if (end < 0) return set_string(circa_output(stack, 0), "");

    if ((unsigned) start > s.length())
        start = s.length();

    if ((unsigned) end > s.length())
        end = s.length();

    if (end < start)
        return set_string(circa_output(stack, 0), "");

    set_string(circa_output(stack, 0), s.substr(start, end - start));
}

void String__ends_with(caStack* stack)
{
    set_bool(circa_output(stack, 0), string_ends_with(circa_input(stack, 0), as_cstring(circa_input(stack, 1))));
}
void String__starts_with(caStack* stack)
{
    set_bool(circa_output(stack, 0), string_starts_with(circa_input(stack, 0), as_cstring(circa_input(stack, 1))));
}

void String__split(caStack* stack)
{
    string_split(circa_input(stack, 0), string_get(circa_input(stack, 1), 0), circa_output(stack, 0));
}

void Type__name(caStack* stack)
{
    set_string(circa_output(stack, 0), name_to_string(as_type(circa_input(stack, 0))->name));
}

void Type__property(caStack* stack)
{
    Type* type = as_type(circa_input(stack, 0));
    const char* str = as_cstring(circa_input(stack, 1));
    caValue* prop = get_type_property(type, str);
    if (prop == NULL)
        set_null(circa_output(stack, 0));
    else
        copy(prop, circa_output(stack, 0));
}

void Term__name(caStack* stack)
{
    Term* t = circa_input(stack, 0)->asRef();
    if (t == NULL)
        return circa_output_error(stack, "NULL reference");
    set_string(circa_output(stack, 0), t->name);
}
void Term__to_string(caStack* stack)
{
    Term* t = circa_input(stack, 0)->asRef();
    if (t == NULL)
        return circa_output_error(stack, "NULL reference");
    set_string(circa_output(stack, 0), circa::to_string(term_value(t)));
}
void Term__to_source_string(caStack* stack)
{
    Term* t = circa_input(stack, 0)->asRef();
    if (t == NULL)
        return circa_output_error(stack, "NULL reference");
    set_string(circa_output(stack, 0), get_term_source_text(t));
}
void Term__function(caStack* stack)
{
    Term* t = circa_input(stack, 0)->asRef();
    if (t == NULL)
        return circa_output_error(stack, "NULL reference");
    set_function(circa_output(stack, 0), as_function(t->function));
}
void Term__type(caStack* stack)
{
    Term* t = circa_input(stack, 0)->asRef();
    if (t == NULL)
        return circa_output_error(stack, "NULL reference");
    set_type(circa_output(stack, 0), t->type);
}
void Term__assign(caStack* stack)
{
    Term* target = circa_input(stack, 0)->asRef();
    if (target == NULL) {
        circa_output_error(stack, "NULL reference");
        return;
    }

    caValue* source = circa_input(stack, 1);

    if (!cast_possible(source, declared_type(target))) {
        circa_output_error(stack, "Can't assign, type mismatch");
        return;
    }

    circa::copy(source, term_value(target));
}
void Term__value(caStack* stack)
{
    Term* target = circa_input(stack, 0)->asRef();
    if (target == NULL) {
        circa_output_error(stack, "NULL reference");
        return;
    }

    copy(term_value(target), circa_output(stack, 0));
}

int tweak_round(double a) {
    return int(a + 0.5);
}

void Term__tweak(caStack* stack)
{
    Term* t = circa_input(stack, 0)->asRef();
    if (t == NULL)
        return circa_output_error(stack, "NULL reference");

    int steps = tweak_round(circa_input(stack, 1)->toFloat());

    caValue* val = term_value(t);

    if (steps == 0)
        return;

    if (is_float(val)) {
        float step = get_step(t);

        // Do the math like this so that rounding errors are not accumulated
        float new_value = (round(as_float(val) / step) + steps) * step;
        set_float(val, new_value);

    } else if (is_int(val))
        set_int(val, as_int(val) + steps);
    else
        circa_output_error(stack, "Ref is not an int or number");
}

void Term__asint(caStack* stack)
{
    Term* t = circa_input(stack, 0)->asRef();
    if (t == NULL) {
        circa_output_error(stack, "NULL reference");
        return;
    }
    if (!is_int(term_value(t))) {
        circa_output_error(stack, "Not an int");
        return;
    }
    set_int(circa_output(stack, 0), as_int(term_value(t)));
}
void Term__asfloat(caStack* stack)
{
    Term* t = circa_input(stack, 0)->asRef();
    if (t == NULL) {
        circa_output_error(stack, "NULL reference");
        return;
    }
    
    set_float(circa_output(stack, 0), to_float(term_value(t)));
}
void Term__input(caStack* stack)
{
    Term* t = circa_input(stack, 0)->asRef();
    if (t == NULL) {
        circa_output_error(stack, "NULL reference");
        return;
    }
    int index = circa_input(stack, 1)->asInt();
    if (index >= t->numInputs())
        set_ref(circa_output(stack, 0), NULL);
    else
        set_ref(circa_output(stack, 0), t->input(index));
}
void Term__inputs(caStack* stack)
{
    Term* t = circa_input(stack, 0)->asRef();
    if (t == NULL)
        return circa_output_error(stack, "NULL reference");

    caValue* output = circa_output(stack, 0);
    circa_set_list(output, t->numInputs());

    for (int i=0; i < t->numInputs(); i++)
        set_ref(circa_index(output, i), t->input(i));
}
void Term__num_inputs(caStack* stack)
{
    Term* t = circa_input(stack, 0)->asRef();
    if (t == NULL) {
        circa_output_error(stack, "NULL reference");
        return;
    }
    set_int(circa_output(stack, 0), t->numInputs());
}

void Term__source_location(caStack* stack)
{
    Term* t = circa_input(stack, 0)->asRef();
    if (t == NULL)
        return circa_output_error(stack, "NULL reference");

    circa_set_vec4(circa_output(stack, 0),
        t->sourceLoc.col, t->sourceLoc.line,
        t->sourceLoc.colEnd, t->sourceLoc.lineEnd);
}
void Term__global_id(caStack* stack)
{
    Term* t = circa_input(stack, 0)->asRef();
    if (t == NULL)
        return circa_output_error(stack, "NULL reference");

    set_int(circa_output(stack, 0), t->id);
}
void Term__properties(caStack* stack)
{
    Term* t = circa_input(stack, 0)->asRef();
    if (t == NULL)
        return circa_output_error(stack, "NULL reference");
    circa::copy(&t->properties, circa_output(stack, 0));
}
void Term__property(caStack* stack)
{
    Term* t = circa_input(stack, 0)->asRef();
    if (t == NULL)
        return circa_output_error(stack, "NULL reference");

    const char* key = circa_string_input(stack, 1);

    caValue* value = term_get_property(t, key);

    if (value == NULL)
        set_null(circa_output(stack, 0));
    else
        circa::copy(value, circa_output(stack, 0));
}

void length(caStack* stack)
{
    set_int(circa_output(stack, 0), num_elements(circa_input(stack, 0)));
}

std::string stackVariable_toString(caValue* value)
{
    short relativeFrame = value->value_data.asint >> 16;
    short index = (value->value_data.asint & 0xffff);
    std::stringstream strm;
    strm << "[frame:" << relativeFrame << ", index:" << index << "]";
    return strm.str();
}

Branch* kernel()
{
    return KERNEL;
}

void create_primitive_types()
{
    null_t::setup_type(&NULL_T);
    bool_t::setup_type(&BOOL_T);
    branch_setup_type(&BRANCH_T);
    dict_t::setup_type(&DICT_T);
    eval_context_t::setup_type(&EVAL_CONTEXT_T);
    number_t::setup_type(&FLOAT_T);
    // handle_t::setup_type(&HANDLE_T);
    int_t::setup_type(&INT_T);
    list_t::setup_type(&LIST_T);
    name_t::setup_type(&NAME_T);
    opaque_pointer_t::setup_type(&OPAQUE_POINTER_T);
    ref_t::setup_type(&REF_T);
    string_setup_type(&STRING_T);
    void_t::setup_type(&VOID_T);
    eval_context_setup_type(&EVAL_CONTEXT_T);

    // errors are just stored as strings for now
    string_setup_type(&ERROR_T);
}

void bootstrap_kernel()
{
    // Create the very first building blocks. These elements need to be in place
    // before we can parse code in the proper way.

    KERNEL = new Branch();
    Branch* kernel = KERNEL;

    // Create value function
    Term* valueFunc = kernel->appendNew();
    rename(valueFunc, "value");
    FUNCS.value = valueFunc;

    // Create Type type
    TYPE_TYPE = kernel->appendNew();
    TYPE_TYPE->function = FUNCS.value;
    TYPE_TYPE->type = &TYPE_T;
    term_value(TYPE_TYPE)->value_type = &TYPE_T;
    term_value(TYPE_TYPE)->value_data.ptr = &TYPE_T;
    type_t::setup_type(&TYPE_T);
    rename(TYPE_TYPE, "Type");

    // Create Any type
    ANY_TYPE = kernel->appendNew();
    ANY_TYPE->function = valueFunc;
    ANY_TYPE->type = &TYPE_T;
    term_value(ANY_TYPE)->value_type = &TYPE_T;
    term_value(ANY_TYPE)->value_data.ptr = &ANY_T;
    any_t::setup_type(&ANY_T);
    rename(ANY_TYPE, "any");

    // Create Function type
    function_t::setup_type(&FUNCTION_T);
    FUNCTION_TYPE = kernel->appendNew();
    FUNCTION_TYPE->function = valueFunc;
    FUNCTION_TYPE->type = &TYPE_T;
    term_value(FUNCTION_TYPE)->value_type = &TYPE_T;
    term_value(FUNCTION_TYPE)->value_data.ptr = &FUNCTION_T;
    rename(FUNCTION_TYPE, "Function");

    // Initialize value() func
    valueFunc->type = &FUNCTION_T;
    valueFunc->function = valueFunc;
    create(&FUNCTION_T, term_value(valueFunc));

    function_t::initialize(&FUNCTION_T, term_value(valueFunc));
    initialize_function(valueFunc);
    as_function(valueFunc)->name = "value";
    function_set_empty_evaluation(as_function(valueFunc));

    // Initialize primitive types (this requires value() function)
    BOOL_TYPE = create_type_value(kernel, &BOOL_T, "bool");
    FLOAT_TYPE = create_type_value(kernel, &FLOAT_T, "number");
    INT_TYPE = create_type_value(kernel, &INT_T, "int");
    NAME_TYPE = create_type_value(kernel, &NAME_T, "Name");
    NULL_T_TERM = create_type_value(kernel, &NULL_T, "Null");
    STRING_TYPE = create_type_value(kernel, &STRING_T, "String");
    DICT_TYPE = create_type_value(kernel, &DICT_T, "Dict");
    REF_TYPE = create_type_value(kernel, &REF_T, "Term");
    VOID_TYPE = create_type_value(kernel, &VOID_T, "void");
    LIST_TYPE = create_type_value(kernel, &LIST_T, "List");
    OPAQUE_POINTER_TYPE = create_type_value(kernel, &OPAQUE_POINTER_T, "opaque_pointer");
    create_type_value(kernel, &BRANCH_T, "Branch");

    // Setup output_placeholder() function, needed to declare functions properly.
    FUNCS.output = create_value(kernel, &FUNCTION_T, "output_placeholder");
    function_t::initialize(&FUNCTION_T, term_value(FUNCS.output));
    initialize_function(FUNCS.output);
    as_function(FUNCS.output)->name = "output_placeholder";
    as_function(FUNCS.output)->evaluate = NULL;
    as_function(FUNCS.output)->specializeType = output_placeholder_specializeType;
    ca_assert(function_get_output_type(FUNCS.output, 0) == &ANY_T);

    // Fix some holes in value() function
    Function* attrs = as_function(valueFunc);
    finish_building_function(attrs, &ANY_T);

    ca_assert(function_get_output_type(valueFunc, 0) == &ANY_T);

    // input_placeholder() is needed before we can declare a function with inputs
    FUNCS.input = import_function(kernel, NULL, "input_placeholder() -> any");
    function_set_empty_evaluation(as_function(FUNCS.input));

    // Now that we have input_placeholder() let's declare one input on output_placeholder()
    apply(function_contents(as_function(FUNCS.output)),
        FUNCS.input, TermList())->setBoolProp("optional", true);

    // FileSignature is used in some builtin functions
    TYPES.file_signature = unbox_type(parse_type(kernel,
            "type FileSignature { String filename, int time_modified }"));

    namespace_function::early_setup(kernel);

    // Set up some global constants
    set_bool(&TrueValue, true);
    set_bool(&FalseValue, false);

    FINISHED_BOOTSTRAP = true;

    // Initialize compound types
    Term* set_type = parse_type(kernel, "type Set;");
    set_t::setup_type(unbox_type(set_type));

    Term* map_type = parse_type(kernel, "type Map;");
    hashtable_t::setup_type(unbox_type(map_type));

    Term* styledSourceType = parse_type(kernel, "type StyledSource;");
    styled_source_t::setup_type(unbox_type(styledSourceType));

    Term* indexableType = parse_type(kernel, "type Indexable;");
    indexable_t::setup_type(unbox_type(indexableType));

    callable_t::setup_type(unbox_type(parse_type(kernel, "type Callable;")));

    RECT_I_TYPE_TERM = parse_type(kernel, "type Rect_i { int x1, int y1, int x2, int y2 }");

    // Setup all the builtin functions defined in src/functions
    setup_builtin_functions(kernel);

    // Create IMPLICIT_TYPES (deprecated)
    type_initialize_kernel(kernel);

    // Now we can build derived functions

    // Create overloaded functions
    FUNCS.add = create_overloaded_function(kernel, "add(any,any) -> any");
    append_to_overloaded_function(FUNCS.add, FUNCS.add_i);
    append_to_overloaded_function(FUNCS.add, FUNCS.add_f);

    Term* less_than = create_overloaded_function(kernel, "less_than(any,any) -> bool");
    append_to_overloaded_function(less_than, kernel->get("less_than_i"));
    append_to_overloaded_function(less_than, kernel->get("less_than_f"));

    Term* less_than_eq = create_overloaded_function(kernel, "less_than_eq(any,any) -> bool");
    append_to_overloaded_function(less_than_eq, kernel->get("less_than_eq_i"));
    append_to_overloaded_function(less_than_eq, kernel->get("less_than_eq_f"));

    Term* greater_than = create_overloaded_function(kernel, "greater_than(any,any) -> bool");
    append_to_overloaded_function(greater_than, kernel->get("greater_than_i"));
    append_to_overloaded_function(greater_than, kernel->get("greater_than_f"));

    Term* greater_than_eq = create_overloaded_function(kernel, "greater_than_eq(any,any) -> bool");
    append_to_overloaded_function(greater_than_eq, kernel->get("greater_than_eq_i"));
    append_to_overloaded_function(greater_than_eq, kernel->get("greater_than_eq_f"));

    Term* max_func = create_overloaded_function(kernel, "max(any,any) -> any");
    append_to_overloaded_function(max_func, kernel->get("max_i"));
    append_to_overloaded_function(max_func, kernel->get("max_f"));

    Term* min_func = create_overloaded_function(kernel, "min(any,any) -> any");
    append_to_overloaded_function(min_func, kernel->get("min_i"));
    append_to_overloaded_function(min_func, kernel->get("min_f"));

    Term* remainder_func = create_overloaded_function(kernel, "remainder(any,any) -> any");
    append_to_overloaded_function(remainder_func, kernel->get("remainder_i"));
    append_to_overloaded_function(remainder_func, kernel->get("remainder_f"));

    Term* mod_func = create_overloaded_function(kernel, "max(any,any) -> any");
    append_to_overloaded_function(mod_func, kernel->get("max_i"));
    append_to_overloaded_function(mod_func, kernel->get("max_f"));

    FUNCS.mult = create_overloaded_function(kernel, "mult(any,any) -> any");
    //dump(nested_contents(FUNCS.mult));
    append_to_overloaded_function(FUNCS.mult, kernel->get("mult_i"));
    append_to_overloaded_function(FUNCS.mult, kernel->get("mult_f"));

    FUNCS.neg = create_overloaded_function(kernel, "neg(any) -> any");
    append_to_overloaded_function(FUNCS.neg, kernel->get("neg_i"));
    append_to_overloaded_function(FUNCS.neg, kernel->get("neg_f"));
    as_function(FUNCS.neg)->formatSource = neg_function::formatSource;

    FUNCS.sub = create_overloaded_function(kernel, "sub(any,any) -> any");
    append_to_overloaded_function(FUNCS.sub, kernel->get("sub_i"));
    append_to_overloaded_function(FUNCS.sub, kernel->get("sub_f"));

    // Create vectorized functions
    Term* add_v = create_function(kernel, "add_v");
    create_function_vectorized_vv(function_contents(add_v), FUNCS.add, &LIST_T, &LIST_T);
    Term* add_s = create_function(kernel, "add_s");
    create_function_vectorized_vs(function_contents(add_s), FUNCS.add, &LIST_T, &ANY_T);

    append_to_overloaded_function(FUNCS.add, add_v);
    append_to_overloaded_function(FUNCS.add, add_s);

    //dump(function_contents(add_v));
    //dump(function_contents(FUNCS.add));

    Term* sub_v = create_function(kernel, "sub_v");
    create_function_vectorized_vv(function_contents(sub_v), FUNCS.sub, &LIST_T, &LIST_T);
    Term* sub_s = create_function(kernel, "sub_s");
    create_function_vectorized_vs(function_contents(sub_s), FUNCS.sub, &LIST_T, &ANY_T);
    
    append_to_overloaded_function(FUNCS.sub, sub_v);
    append_to_overloaded_function(FUNCS.sub, sub_s);

    // Create vectorized mult() functions
    Term* mult_v = create_function(kernel, "mult_v");
    create_function_vectorized_vv(function_contents(mult_v), FUNCS.mult, &LIST_T, &LIST_T);
    Term* mult_s = create_function(kernel, "mult_s");
    create_function_vectorized_vs(function_contents(mult_s), FUNCS.mult, &LIST_T, &ANY_T);

    append_to_overloaded_function(FUNCS.mult, mult_v);
    append_to_overloaded_function(FUNCS.mult, mult_s);

    Term* div_s = create_function(kernel, "div_s");
    create_function_vectorized_vs(function_contents(div_s), FUNCS.div, &LIST_T, &ANY_T);

    // Create some hosted types
    TYPES.point = as_type(term_value(parse_type(kernel, "type Point { number x, number y }")));
    parse_type(kernel, "type Point_i { int x, int y }");
    parse_type(kernel, "type Rect { number x1, number y1, number x2, number y2 }");

    TYPES.color = unbox_type(parse_type(kernel,
                    "type Color { number r, number g, number b, number a }"));

    color_t::setup_type(TYPES.color);

    // Need dynamic_method before any hosted functions
    FUNCS.dynamic_method = import_function(KERNEL, dynamic_method_call, "def dynamic_method(any inputs :multiple) -> any");
}

void install_standard_library(Branch* kernel)
{
    // Parse the stdlib script
    parser::compile(kernel, parser::statement_list, STDLIB_CA_TEXT);

    // Install C functions
    static const ImportRecord records[] = {
        {"cppbuild:build_module", cppbuild_function::build_module},
        {"file:version", file__version},
        {"file:exists", file__exists},
        {"file:read_text", file__read_text},
        {"file:fetch_record", file__fetch_record},
        {"length", length},
        {"from_string", from_string},
        {"to_string_repr", to_string_repr},
        {"call", (EvaluateFunc) dynamic_call_func},
        {"refactor:rename", refactor__rename},
        {"refactor:change_function", refactor__change_function},
        {"reflect:this_branch", reflect__this_branch},
        {"reflect:kernel", reflect__kernel},
        {"sys:module_search_paths", sys__module_search_paths},
        {"sys:do_admin_command", sys__do_admin_command},
        {"branch_ref", branch_ref},
        {"load_script_value", load_script_value},
        {"Branch.input", Branch__input},
        {"Branch.inputs", Branch__inputs},
        {"Branch.output", Branch__output},
        {"Branch.outputs", Branch__outputs},
        {"Branch.dump", Branch__dump},
        {"Branch.evaluate", Branch__evaluate},
        {"Branch.file_signature", Branch__file_signature},
        {"Branch.statements", (EvaluateFunc) Branch__statements},
        {"Branch.format_source", Branch__format_source},
        {"Branch.get_term", Branch__get_term},
        {"Branch.get_static_errors", Branch__get_static_errors},
        {"Branch.get_static_errors_formatted", Branch__get_static_errors_formatted},
        {"Branch.has_static_error", Branch__has_static_error},
        {"Branch.list_configs", Branch__list_configs},
        {"Branch.functions", Branch__functions},
        {"Branch.terms", Branch__terms},
        {"Function.name", Function__name},
        {"Function.input", Function__input},
        {"Function.inputs", Function__inputs},
        {"Function.output", Function__output},
        {"Function.outputs", Function__outputs},
        {"Function.contents", Function__contents},

        {"String.char_at", String__char_at},
        {"String.ends_with", String__ends_with},
        {"String.length", String__length},
        {"String.substr", String__substr},
        {"String.slice", String__slice},
        {"String.starts_with", String__starts_with},
        {"String.split", String__split},
        {"String.to_camel_case", String__to_camel_case},
        {"String.to_upper", String__to_upper},
        {"String.to_lower", String__to_lower},
        
        {"Type.name", Type__name},
        {"Type.property", Type__property},

        {"Term.name", Term__name},
        {"Term.to_string", Term__to_string},
        {"Term.to_source_string", Term__to_source_string},
        {"Term.function", Term__function},
        {"Term.get_type", Term__type},
        {"Term.assign", Term__assign},
        {"Term.value", Term__value},
        {"Term.tweak", Term__tweak},
        {"Term.asint", Term__asint},
        {"Term.asfloat", Term__asfloat},
        {"Term.input", Term__input},
        {"Term.inputs", Term__inputs},
        {"Term.num_inputs", Term__num_inputs},
        {"Term.source_location", Term__source_location},
        {"Term.global_id", Term__global_id},
        {"Term.properties", Term__properties},
        {"Term.property", Term__property},
        {NULL, NULL}
    };

    install_function_list(kernel, records);

    FUNCS.dll_patch = kernel->get("sys:dll_patch");
    FUNCS.length = kernel->get("length");
    FUNCS.type = kernel->get("type");
    FUNCS.input_explicit = kernel->get("input");
    FUNCS.output_explicit = kernel->get("input");
}

EXPORT caWorld* circa_initialize()
{
    caWorld* world = (caWorld*) malloc(sizeof(world));

    FINISHED_BOOTSTRAP = false;
    STATIC_INITIALIZATION_FINISHED = true;

    memset(&FUNCS, 0, sizeof(FUNCS));
    memset(&TYPES, 0, sizeof(TYPES));

    create_primitive_types();
    bootstrap_kernel();

    Branch* kernel = KERNEL;

    install_standard_library(kernel);

    // Make sure there are no static errors. This shouldn't happen.
    if (has_static_errors(kernel)) {
        std::cout << "Static errors found in kernel:" << std::endl;
        print_static_errors_formatted(kernel, std::cout);
        internal_error("circa fatal: static errors found in kernel");
    }

#if CIRCA_ENABLE_FILESYSTEM
    // Use standard filesystem by default
    circa_use_standard_filesystem(world);
#endif

    // Load library paths from CIRCA_LIB_PATH
    const char* libPathEnv = getenv("CIRCA_LIB_PATH");
    if (libPathEnv != NULL) {
        Value libPathStr;
        set_string(&libPathStr, libPathEnv);

        Value libPaths;
        string_split(&libPathStr, ';', &libPaths);

        for (int i=0; i < list_length(&libPaths); i++) {
            caValue* path = list_get(&libPaths, i);
            if (string_eq(path, ""))
                continue;
            modules_add_search_path(as_cstring(path));
        }
    }

    return world;
}

EXPORT void circa_shutdown(caWorld* world)
{
    SHUTTING_DOWN = true;

    clear_type_contents(&BOOL_T);
    clear_type_contents(&DICT_T);
    clear_type_contents(&ERROR_T);
    clear_type_contents(&FLOAT_T);
    clear_type_contents(&INT_T);
    clear_type_contents(&LIST_T);
    clear_type_contents(&NULL_T);
    clear_type_contents(&OPAQUE_POINTER_T);
    clear_type_contents(&REF_T);
    clear_type_contents(&STRING_T);
    clear_type_contents(&TYPE_T);
    clear_type_contents(&VOID_T);

    delete KERNEL;
    KERNEL = NULL;

    memset(&FUNCS, 0, sizeof(FUNCS));

    name_dealloc_global_data();

    gc_collect();

    free(world);
}

} // namespace circa

// Public API

using namespace circa;

caBranch* circa_kernel(caWorld*)
{
    return KERNEL;
}
