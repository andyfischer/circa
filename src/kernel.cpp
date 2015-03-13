// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa/circa.h"
#include "circa/file.h"

#include "blob.h"
#include "block.h"
#include "building.h"
#include "builtin_types.h"
#include "closures.h"
#include "control_flow.h"
#include "code_iterators.h"
#include "function.h"
#include "hashtable.h"
#include "inspection.h"
#include "kernel.h"
#include "list.h"
#include "modules.h"
#include "misc_builtins.h"
#include "native_patch.h"
#include "native_ptr.h"
#include "parser.h"
#include "reflection.h"
#include "selector.h"
#include "static_checking.h"
#include "string_repr.h"
#include "string_type.h"
#include "symbols.h"
#include "names.h"
#include "term.h"
#include "type_inference.h"
#include "type.h"
#include "world.h"
#include "vm.h"

namespace circa {

World* g_world = NULL;

BuiltinFuncs FUNCS;
BuiltinTypes TYPES;

Term* find_builtin_func(Block* builtins, const char* name);

Type* output_placeholder_specializeType(Term* caller)
{
    // Don't specialize if the type was explicitly declared
    if (caller->boolProp(s_ExplicitType, false))
        return declared_type(caller);

    // Special case: if we're an accumulatingOutput then the output type is List.
    if (caller->boolProp(s_AccumulatingOutput, false))
        return TYPES.list;

    if (caller->input(0) == NULL)
        return NULL;

    return declared_type(caller->input(0));
}

std::string stackVariable_toString(Value* value)
{
    short relativeFrame = value->value_data.asint >> 16;
    short index = (value->value_data.asint & 0xffff);
    std::stringstream strm;
    strm << "[frame:" << relativeFrame << ", index:" << index << "]";
    return strm.str();
}

Type* specializeType_add_sub_mult(Term* term)
{
    Type* left = declared_type(term->input(0));
    Type* right = declared_type(term->input(1));

    if (left == TYPES.int_type && right == TYPES.int_type)
        return TYPES.int_type;

    if ((left == TYPES.int_type || left == TYPES.float_type)
            && (right == TYPES.int_type || right == TYPES.float_type))
        return TYPES.float_type;

    return TYPES.any;
}

Type* specializeType_div(Term* term)
{
    Type* left = declared_type(term->input(0));
    Type* right = declared_type(term->input(1));

    if ((left == TYPES.int_type || left == TYPES.float_type)
            && (right == TYPES.int_type || right == TYPES.float_type))
        return TYPES.float_type;

    return TYPES.any;
}

Type* cond_specializeType(Term* caller)
{
    Value choices;
    Type* leftType = caller->input(1) != NULL ? caller->input(1)->type : NULL;
    Type* rightType = caller->input(2) != NULL ? caller->input(2)->type : NULL;
    set_type_list(&choices, leftType, rightType);
    return find_common_type(&choices);
}

Type* cast_specializeType(Term* caller)
{
    Term* input = caller->input(0);
    if (input == NULL)
        return TYPES.any;

    if (is_value(input) && is_type(input))
        return as_type(input);

    return TYPES.any;
}

Type* copy_specializeType(Term* caller)
{
    return get_type_of_input(caller, 0);
}

Type* get_field_specializeType(Term* caller)
{
    Type* head = caller->input(0)->type;

    for (int nameIndex=1; nameIndex < caller->numInputs(); nameIndex++) {

        // Abort if input type is not correct
        if (!is_string(term_value(caller->input(1))))
            return TYPES.any;

        if (!is_list_based_type(head))
            return TYPES.any;

        std::string const& name = as_string(term_value(caller->input(1)));

        int fieldIndex = list_find_field_index_by_name(head, name.c_str());

        if (fieldIndex == -1)
            return TYPES.any;

        head = as_type(get_index(list_get_type_list_from_type(head),fieldIndex));
    }

    return head;
}

Type* get_index_specializeType(Term* term)
{
    return infer_type_of_get_index(term->input(0));
}

Type* make_specializeType(Term* caller)
{
    Term* input = caller->input(0);
    if (input == NULL)
        return TYPES.any;

    if (is_value(input) && is_type(input))
        return as_type(input);

    return TYPES.any;
}

Type* range_specializeType(Term* term)
{
    Type* type = create_typed_unsized_list_type(TYPES.int_type);
    type_start_at_zero_refs(type);
    return type;
}

Type* set_field_specializeType(Term* caller)
{
    return caller->input(0)->type;
}

Type* set_index_specializeType(Term* caller)
{
    // TODO: Fix type inference on set_index.
    return TYPES.list;
    //return caller->input(0)->type;
}

Type* extra_output_specializeType(Term* term)
{
    ca_assert(term->input(0)->owningBlock == term->owningBlock);
    int myOutputIndex = term->index - term->input(0)->index;
    return get_output_type(term->input(0), myOutputIndex);
}

Type* List__append_specializeType(Term* term)
{
    Term* listInput = term->input(0);
    switch (list_get_parameter_type(&listInput->type->parameter)) {
    case s_Untyped:
        return listInput->type;
    case s_UniformListType:
    {
        Type* listElementType = list_get_repeated_type_from_type(listInput->type);
        Type* commonType = find_common_type(listElementType, term->input(1)->type);
        if (commonType == listElementType)
            return listInput->type;
        else
            return create_typed_unsized_list_type(commonType);
    }
    case s_AnonStructType:
    case s_StructType:
    {    
        Value elementTypes;
        copy(list_get_type_list_from_type(listInput->type), &elementTypes);
        set_type(elementTypes.append(), term->input(1)->type);
        return create_typed_unsized_list_type(find_common_type(&elementTypes));
    }
    case s_invalid:
    default:
        return TYPES.any;
    }
}

Type* Type_cast_specializeType(Term* caller)
{
    if (is_value(caller->input(0)) && is_type(caller->input(0)))
        return as_type(term_value(caller->input(0)));

    return NULL;
}

Type* type_make_specializeType(Term* caller)
{
    Term* input = caller->input(0);
    if (input == NULL)
        return TYPES.any;

    if (is_value(input) && is_type(input))
        return as_type(input);

    return TYPES.any;
}

World* global_world()
{
    return g_world;
}

Block* global_builtins_block()
{
    return global_world()->builtins;
}

void term_toString(Value* val, Value* out)
{
    Term* t = as_term_ref(val);
    if (t == NULL)
        string_append(out, "Term#null");
    else {
        string_append(out, "Term#");
        string_append(out, t->id);
    }
}

int term_hashFunc(Value* val)
{
    Term* term = as_term_ref(val);
    if (term == NULL)
        return 0;
    return term->id;
}

void term_setup_type(Type* type)
{
    set_string(&type->name, "Term");
    type->storageType = s_StorageTypeTerm;
    type->toString = term_toString;
    type->hashFunc = term_hashFunc;
}

// Spy & oracle
void for_each_root_type(void (*callback)(Type* type))
{
    (*callback)(TYPES.any);
    (*callback)(TYPES.blob);
    (*callback)(TYPES.block);
    (*callback)(TYPES.bool_type);
    (*callback)(TYPES.error);
    (*callback)(TYPES.float_type);
    (*callback)(TYPES.int_type);
    (*callback)(TYPES.list);
    (*callback)(TYPES.map);
    (*callback)(TYPES.opaque_pointer);
    (*callback)(TYPES.native_ptr);
    (*callback)(TYPES.null);
    (*callback)(TYPES.string);
    (*callback)(TYPES.symbol);
    (*callback)(TYPES.term);
    (*callback)(TYPES.type);
    (*callback)(TYPES.vm);
    (*callback)(TYPES.void_type);
}

void bootstrap_kernel()
{
    memset(&FUNCS, 0, sizeof(FUNCS));
    memset(&TYPES, 0, sizeof(TYPES));

    // Allocate a World object.
    g_world = alloc_world();
    g_world->bootstrapStatus = s_Bootstrapping;
    World* world = g_world;

    // Instanciate the types that are used by Type.
    TYPES.map = create_type_unconstructed();
    TYPES.null = create_type_unconstructed();
    TYPES.string = create_type_unconstructed();
    TYPES.type = create_type_unconstructed();

    // Now we can fully instanciate types.
    type_finish_construction(TYPES.map);
    type_finish_construction(TYPES.null);
    type_finish_construction(TYPES.string);
    type_finish_construction(TYPES.type);
    string_setup_type(TYPES.string);

    // Initialize remaining global types.
    TYPES.any = create_type();
    TYPES.blob = create_type();
    TYPES.block = create_type();
    TYPES.bool_type = create_type();
    TYPES.error = create_type();
    TYPES.float_type = create_type();
    TYPES.int_type = create_type();
    TYPES.list = create_type();
    TYPES.native_ptr = create_type();
    TYPES.map = create_type();
    TYPES.opaque_pointer = create_type();
    TYPES.symbol = create_type();
    TYPES.term = create_type();
    TYPES.vm = create_type();
    TYPES.void_type = create_type();

    for_each_root_type(type_set_root);

    any_setup_type(TYPES.any);
    blob_setup_type(TYPES.blob);
    block_setup_type(TYPES.block);
    bool_setup_type(TYPES.bool_type);
    hashtable_setup_type(TYPES.map);
    int_setup_type(TYPES.int_type);
    list_t::setup_type(TYPES.list);
    symbol_setup_type(TYPES.symbol);
    native_ptr_setup_type(TYPES.native_ptr);
    null_setup_type(TYPES.null);
    number_setup_type(TYPES.float_type);
    opaque_pointer_setup_type(TYPES.opaque_pointer);
    term_setup_type(TYPES.term);
    string_setup_type(TYPES.error); // errors are just stored as strings for now
    type_t::setup_type(TYPES.type);
    void_setup_type(TYPES.void_type);
    vm_setup_type(TYPES.vm);

    // Finish initializing World (this requires List and Hashtable types)
    world_initialize(g_world);

    // Create builtins block.
    Value builtinsStr;
    set_string(&builtinsStr, "builtins");
    Block* builtins = create_module(g_world);
    module_set_name(world, builtins, &builtinsStr);
    g_world->builtins = builtins;

    // Create function_decl function.
    Term* functionDeclFunction = builtins->appendNew();
    rename(functionDeclFunction, "function_decl");
    FUNCS.function_decl = functionDeclFunction;
    FUNCS.function_decl->function = FUNCS.function_decl;
    make_nested_contents(FUNCS.function_decl);
    block_set_function_has_nested(nested_contents(FUNCS.function_decl), true);

    // Create value function
    Term* valueFunc = builtins->appendNew();
    rename(valueFunc, "value");
    FUNCS.value = valueFunc;

    // Create Type type
    Term* typeType = builtins->appendNew();
    typeType->function = FUNCS.value;
    typeType->type = TYPES.type;
    term_value(typeType)->value_type = TYPES.type;
    term_value(typeType)->value_data.ptr = TYPES.type;
    TYPES.type->declaringTerm = typeType;
    rename(typeType, "Type");

    // Create Any type
    Term* anyType = builtins->appendNew();
    anyType->function = valueFunc;
    anyType->type = TYPES.type;
    term_value(anyType)->value_type = TYPES.type;
    term_value(anyType)->value_data.ptr = TYPES.any;
    TYPES.any->declaringTerm = anyType;
    rename(anyType, "any");

    // Initialize value() func
    valueFunc->type = TYPES.any;
    valueFunc->function = FUNCS.function_decl;
    make_nested_contents(valueFunc);
    block_set_evaluation_empty(nested_contents(valueFunc), true);

    // Initialize primitive types (this requires value() function)
    create_type_value(builtins, TYPES.blob, "Blob");
    create_type_value(builtins, TYPES.bool_type, "bool");
    create_type_value(builtins, TYPES.block, "Block");
    create_type_value(builtins, TYPES.float_type, "number");
    create_type_value(builtins, TYPES.int_type, "int");
    create_type_value(builtins, TYPES.list, "List");
    create_type_value(builtins, TYPES.opaque_pointer, "opaque_pointer");
    create_type_value(builtins, TYPES.native_ptr, "native_ptr");
    create_type_value(builtins, TYPES.string, "String");
    create_type_value(builtins, TYPES.symbol, "Symbol");
    create_type_value(builtins, TYPES.term, "Term");
    create_type_value(builtins, TYPES.map, "Map");
    create_type_value(builtins, TYPES.void_type, "void");
    create_type_value(builtins, TYPES.vm, "VM");

    // Create global symbol table (requires Hashtable type)
    symbol_initialize_global_table();

    // Setup output_placeholder() function, needed to declare functions properly.
    FUNCS.output = apply(builtins, FUNCS.function_decl, TermList(), "output_placeholder");
    nested_contents(FUNCS.output)->overrides.specializeType = output_placeholder_specializeType;
    ca_assert(get_output_type(nested_contents(FUNCS.output), 0) == TYPES.any);

    // Now that output_placeholder is created, fix the value() function.
    {
        Term* output = append_output_placeholder(nested_contents(valueFunc), NULL);
        set_declared_type(output, TYPES.any);
        finish_building_function(nested_contents(valueFunc));
    }

    ca_assert(get_output_type(nested_contents(valueFunc), 0) == TYPES.any);

    // input_placeholder() is needed before we can declare a function with inputs
    FUNCS.input = apply(builtins, FUNCS.function_decl, TermList(), "input_placeholder");
    block_set_evaluation_empty(nested_contents(FUNCS.input), true);

    // Now that we have input_placeholder(), declare one input on output_placeholder()
    apply(nested_contents(FUNCS.output),
        FUNCS.input, TermList())->setBoolProp(s_Optional, true);

    // Initialize a few more types
    TYPES.selector = unbox_type(create_value(builtins, TYPES.type, "Selector"));
    list_t::setup_type(TYPES.selector);

    // Need the comment() function before parsing stdlib.ca
    FUNCS.comment = apply(builtins, FUNCS.function_decl, TermList(), "comment");

    // Parse stdlib.ca
    parse(builtins, parse_statement_list, find_builtin_file("$builtins/stdlib.ca"));
    set_string(block_insert_property(builtins, s_ModuleName), "stdlib");

    blob_install_functions(world->builtinPatch);
    selector_setup_funcs(world->builtinPatch);
    closures_install_functions(world->builtinPatch);
    reflection_install_functions(world->builtinPatch);
    misc_builtins_setup_functions(world->builtinPatch);
    type_install_functions(world->builtinPatch);
    vm_install_functions(world->builtinPatch);

    block_set_bool_prop(builtins, s_Builtins, true);

    ca_assert(FUNCS.declared_state != NULL);

    FUNCS.has_effects = builtins->get("has_effects");
    block_set_has_effects(nested_contents(FUNCS.has_effects), true);

    nested_contents(FUNCS.add)->overrides.specializeType = specializeType_add_sub_mult;
    nested_contents(FUNCS.sub)->overrides.specializeType = specializeType_add_sub_mult;
    nested_contents(FUNCS.mult)->overrides.specializeType = specializeType_add_sub_mult;
    nested_contents(FUNCS.div)->overrides.specializeType = specializeType_div;

    FUNCS.get_with_symbol = builtins->get("get_with_symbol");
    FUNCS.length = builtins->get("length");
    FUNCS.list_append = builtins->get("List.append");
    FUNCS.native_patch = builtins->get("native_patch");
    FUNCS.package = builtins->get("package");

    nested_contents(builtins->get("Type.cast"))->overrides.specializeType = Type_cast_specializeType;

    // Finish setting up types that are declared in stdlib.ca.
    TYPES.color = as_type(builtins->get("Color"));
    TYPES.file_signature = as_type(builtins->get("FileSignature"));
    TYPES.func = as_type(builtins->get("Func"));
    TYPES.module_ref = as_type(builtins->get("Module"));
    TYPES.vec2 = as_type(builtins->get("Vec2"));

    // Fix function_decl now that Func type is available.
    {
        set_declared_type(append_output_placeholder(nested_contents(FUNCS.function_decl), NULL),
            TYPES.func);
        set_declared_type(FUNCS.function_decl, TYPES.func);
        finish_building_function(nested_contents(FUNCS.function_decl));
    }

    // Also, now that Func type is available, update all static closures.
    for (BlockIterator it(builtins); it; ++it) {
        Term* term = *it;
        if (is_function(term)) {
            set_declared_type(term, TYPES.func);
            if (term->owningBlock == builtins)
                // Functions at top level must be static closures
                update_static_closure_force(term);
            else
                update_static_closure_if_possible(term);
        }
    }

    nested_contents(FUNCS.list_append)->overrides.specializeType = List__append_specializeType;

    #define set_evaluation_empty(name) block_set_evaluation_empty(nested_contents(FUNCS.name), true)
        set_evaluation_empty(return_func);
        set_evaluation_empty(discard);
        set_evaluation_empty(break_func);
        set_evaluation_empty(continue_func);
        set_evaluation_empty(comment);
        set_evaluation_empty(extra_output);
        set_evaluation_empty(loop_iterator);
        set_evaluation_empty(static_error);
    #undef set_evaluation_empty

    block_link_missing_functions(builtins, builtins);
}

Term* find_builtin_func(Block* builtins, const char* name)
{
    Term* term = builtins->get(name);
    if (term == NULL) {
        printf("Builtin func not found: %s\n", name);
        internal_error("");
    }
    return term;
}

void on_new_function_parsed(Term* func, Value* functionName)
{
    if (global_world()->bootstrapStatus == s_Done)
        return;

    #define find_func(name, sourceName) if (string_equals(functionName, sourceName)) FUNCS.name = func;
        find_func(add_i, "add_i"); find_func(add_f, "add_f");
        find_func(and_func, "and");
        find_func(break_func, "break");
        find_func(blank_list, "blank_list");
        find_func(case_func, "case");
#if 0
        find_func(case_condition_bool, "case_condition_bool");
#endif
        find_func(cast, "cast");
        find_func(cond, "cond");
        find_func(continue_func, "continue");
        find_func(copy, "copy");
        find_func(declare_field, "declare_field");
        find_func(discard, "discard");
        find_func(div_f, "div_f"); find_func(div_i, "div_i");
        find_func(dynamic_method, "dynamic_method");
        find_func(error, "error");
        find_func(equals, "equals");
        find_func(extra_output, "extra_output");
        find_func(for_func, "for");
        find_func(func_call_implicit, "func_call_implicit");
        find_func(function_decl, "function_decl");
        find_func(get_field, "get_field");
        find_func(get_index, "get_index");
        find_func(get_with_selector, "get_with_selector");
        find_func(greater_than, "greater_than");
        find_func(greater_than_eq, "greater_than_eq");
        find_func(has_effects, "has_effects");
        find_func(if_block, "if");
        find_func(inputs_fit_function, "inputs_fit_function");
        find_func(length, "length");
        find_func(less_than, "less_than");
        find_func(less_than_eq, "less_than_eq");
        find_func(loop_condition_bool, "loop_condition_bool");
        find_func(loop_iterator, "loop_iterator");
        find_func(map, "map");
        find_func(make, "make");
        find_func(make_list, "make_list");
        find_func(method_lookup, "method_lookup");
        find_func(native_patch, "native_patch");
        find_func(neg, "neg");
        find_func(not_equals, "not_equals");
        find_func(not_func, "not");
        find_func(or_func, "or");
        find_func(overload_error_no_match, "overload_error_no_match");
        find_func(range, "range");
        find_func(return_func, "return");
        find_func(remainder, "remainder");
        find_func(require, "require");
        find_func(require_check, "require_check");
        find_func(require_local, "require_local");
        find_func(section_block, "section");
        find_func(selector, "selector");
        find_func(set_index, "set_index");
        find_func(set_field, "set_field");
        find_func(set_with_selector, "set_with_selector");
        find_func(static_error, "static_error");
        find_func(sub_i, "sub_i"); find_func(sub_f, "sub_f");
        find_func(switch_func, "switch");
        find_func(syntax_error, "syntax_error");
        find_func(to_seq, "to_seq");
        find_func(type, "type");
        find_func(unknown_function, "unknown_function");
        find_func(unknown_function_prelude, "unknown_function_prelude");
        find_func(unknown_identifier, "unknown_identifier");
        find_func(while_loop, "while");
        find_func(add, "add");
        find_func(div, "div");
        find_func(mult, "mult");
        find_func(sub, "sub");
        find_func(closure_block, "closure_block");
        find_func(declared_state, "_declared_state");
        find_func(dynamic_method, "dynamic_method");
        find_func(dynamic_term_eval, "_dynamic_term_eval");
        find_func(equals, "equals");
        find_func(error, "error");
        find_func(eval_on_demand, "_eval_on_demand");
        find_func(func_call, "call");
        find_func(func_call_method, "Func.call");
        find_func(func_apply, "func_apply");
        find_func(func_apply_method, "Func.apply");
        find_func(get_with_selector, "get_with_selector");
        find_func(map_get, "Map.get");
        find_func(module_get, "Module._get");
        find_func(not_equals, "not_equals");
        find_func(selector, "selector");
        find_func(set_with_selector, "set_with_selector");
        find_func(type_make, "Type.make");
        find_func(upvalue, "_upvalue");
        find_func(vm_save_declared_state, "vm_save_declared_state");
        find_func(vm_close_stateful_minor_frame, "vm_close_stateful_minor_frame");
    #undef find_func

    #define has_custom_type_infer(name) \
        if (FUNCS.name != NULL) \
            block_set_specialize_type_func(nested_contents(FUNCS.name), name##_specializeType)

        has_custom_type_infer(cond);
        has_custom_type_infer(copy);
        has_custom_type_infer(extra_output);
        has_custom_type_infer(get_field);
        has_custom_type_infer(get_index);
        has_custom_type_infer(make);
        has_custom_type_infer(range);
        has_custom_type_infer(set_field);
        has_custom_type_infer(set_index);
        has_custom_type_infer(type_make);

    #undef has_custom_type_infer

    #define has_post_compile(sourceName, f) if (string_equals(functionName, sourceName)) \
        block_set_post_compile_func(nested_contents(func), f);

        has_post_compile("return", controlFlow_postCompile);
        has_post_compile("discard", controlFlow_postCompile);
        has_post_compile("break", controlFlow_postCompile);
        has_post_compile("continue", controlFlow_postCompile);

    #undef has_post_compile
}

CIRCA_EXPORT caWorld* circa_initialize()
{
    bootstrap_kernel();

    caWorld* world = global_world();

    Block* builtins = global_builtins_block();
    //dump(builtins);

    // Make sure there are no static errors in builtins. This shouldn't happen.
    if (has_static_errors(builtins)) {
        std::cout << "Static errors found in kernel:" << std::endl;
        dump(builtins);
        Value msg;
        print_static_errors_formatted(builtins, &msg);
        dump(&msg);
    }

    // Load library paths from CIRCA_LIB_PATH
    const char* libPathEnv = getenv("CIRCA_LIB_PATH");
    if (libPathEnv != NULL) {
        Value libPathStr;
        set_string(&libPathStr, libPathEnv);

        Value libPaths;
        string_split(&libPathStr, ';', &libPaths);

        for (int i=0; i < list_length(&libPaths); i++) {
            Value* path = list_get(&libPaths, i);
            if (string_equals(path, ""))
                continue;
            module_add_search_path(world, as_cstring(path));
        }
    }

    log_msg(0, "finished circa_initialize");

    world->bootstrapStatus = s_Done;

    return world;
}

CIRCA_EXPORT void circa_shutdown(caWorld* world)
{
    symbol_deinitialize_global_table();

    world_uninitialize(world);

    for_each_root_type(predelete_type);
    for_each_root_type(delete_type);

    memset(&FUNCS, 0, sizeof(FUNCS));
    memset(&TYPES, 0, sizeof(TYPES));

    dealloc_world(world);
}

} // namespace circa
