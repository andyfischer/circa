// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"
#include "circa/circa.h"

#include <set>

#include "block.h"
#include "building.h"
#include "code_iterators.h"
#include "hashtable.h"
#include "inspection.h"
#include "kernel.h"
#include "modules.h"
#include "native_patch.h"
#include "reflection.h"
#include "term.h"
#include "tagged_value.h"
#include "type.h"
#include "vm.h"

#include "value_iterator.h"

namespace circa {

void Block__dump(VM* vm)
{
    dump(as_block(vm->input(0)));
}

void Block__id(VM* vm)
{
    Block* block = as_block(vm->input(0));
    set_int(vm->output(), block->id);
}

void Block__input(VM* vm)
{
    Block* block = as_block(vm->input(0));
    set_term_ref(vm->output(),
        get_input_placeholder(block, vm->input(1)->as_i()));
}
void Block__inputs(VM* vm)
{
    Block* block = as_block(vm->input(0));
    Value* output = vm->output();
    set_list(output, 0);
    for (int i=0;; i++) {
        Term* term = get_input_placeholder(block, i);
        if (term == NULL)
            break;
        set_term_ref(list_append(output), term);
    }
}
void Block__is_null(VM* vm)
{
    Block* block = as_block(vm->input(0));
    set_bool(vm->output(), block == NULL);
}
void Block__is_major(VM* vm)
{
    Block* block = as_block(vm->input(0));
    set_bool(vm->output(), is_major_block(block));
}
void Block__output(VM* vm)
{
    Block* block = as_block(vm->input(0));
    Term* placeholder = get_output_placeholder(block, vm->input(1)->as_i());
    set_term_ref(vm->output(), placeholder->input(0));
}
void Block__output_placeholder(VM* vm)
{
    Block* block = as_block(vm->input(0));
    Term* placeholder = get_output_placeholder(block, vm->input(1)->as_i());
    set_term_ref(vm->output(), placeholder);
}
void Block__outputs(VM* vm)
{
    Block* block = as_block(vm->input(0));
    Value* output = vm->output();
    set_list(output, 0);
    for (int i=0;; i++) {
        Term* term = get_output_placeholder(block, i);
        if (term == NULL)
            break;
        set_term_ref(list_append(output), term);
    }
}
void Block__owner(VM* vm)
{
    Block* block = as_block(vm->input(0));
    if (block == NULL) {
        set_term_ref(vm->output(), NULL);
        return;
    }

    set_term_ref(vm->output(), block->owningTerm);
}

void Block__parent(VM* vm)
{
    Block* block = as_block(vm->input(0));
    if (block == NULL) {
        set_block(vm->output(), NULL);
        return;
    }

    set_block(vm->output(), get_parent_block(block));
}

void Block__property(VM* vm)
{
    Block* block = as_block(vm->input(0));

    if (block == NULL)
        return vm->throw_str("NULL block");

    Value* value = block_get_property(block, as_symbol(vm->input(1)));

    if (value == NULL)
        set_null(vm->output());
    else
        copy(value, vm->output());
}
void Block__properties(VM* vm)
{
    Block* block = as_block(vm->input(0));

    if (block == NULL)
        return vm->throw_str("NULL block");

    if (is_null(&block->properties))
        set_hashtable(vm->output());
    else
        copy(&block->properties, vm->output());
}
void Block__source_filename(VM* vm)
{
    Block* block = as_block(vm->input(0));
    while (block != NULL) {
        Value* filename = block_get_source_filename(block);

        if (filename != NULL) {
            copy(filename, vm->output());
            return;
        }

        block = get_parent_block(block);
    }
    set_string(vm->output(), "");
}

void Block__term_named(VM* vm)
{
    Block* block = as_block(vm->input(0));
    if (block == NULL)
        return vm->throw_str("NULL block");

    Value* name = vm->input(1);

    set_term_ref(vm->output(), find_local_name(block, name));
}

void Block__terms(VM* vm)
{
    Block* block = as_block(vm->input(0));
    if (block == NULL)
        return vm->throw_str("NULL block");

    Value* out = vm->output();
    set_list(out, block->length());

    for (int i=0; i < block->length(); i++)
        set_term_ref(circa_index(out, i), block->get(i));
}

void Block__walk_terms(VM* vm)
{
    Block* block = as_block(vm->input(0));
    if (block == NULL)
        return vm->throw_str("NULL block");

    Value* out = vm->output();
    set_list(out, 0);
    for (BlockIterator it(block); it; ++it)
        set_term_ref(list_append(out), *it);
}

void Block__get_term(VM* vm)
{
    Block* block = as_block(vm->input(0));
    if (block == NULL)
        return vm->throw_str("NULL block");

    int index = vm->input(1)->as_i();
    set_term_ref(vm->output(), block->get(index));
}

bool is_considered_config(Term* term)
{
    if (term == NULL) return false;
    if (has_empty_name(term)) return false;
    if (!is_value(term)) return false;
    if (is_declared_state(term)) return false;
    if (is_hidden(term)) return false;
    if (is_function(term)) return false;
    if (is_type(term)) return false;

    return true;
}

void Block__list_configs(VM* vm)
{
    Block* block = as_block(vm->input(0));
    if (block == NULL)
        return vm->throw_str("NULL block");

    Value* output = vm->output();

    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);
        if (is_considered_config(term))
            set_term_ref(circa_append(output), term);
    }
}

void Block__functions(VM* vm)
{
    Block* block = as_block(vm->input(0));
    if (block == NULL)
        return vm->throw_str("NULL block");

    Value* output = vm->output();
    set_list(output, 0);

    for (BlockIteratorFlat it(block); it.unfinished(); it.advance()) {
        Term* term = *it;
        if (is_function(term)) {
            set_block(list_append(output), nested_contents(term));
        }
    }
}

void Block__find_term(VM* vm)
{
    Block* block = as_block(vm->input(0));
    if (block == NULL)
        return vm->throw_str("NULL block");

    Term* term = block->get(vm->input(1)->as_str());

    set_term_ref(vm->output(), term);
}

void Block__statements(VM* vm)
{
    Block* block = (Block*) circa_block(vm->input(0));
    if (block == NULL)
        return vm->throw_str("NULL block");

    Value* out = vm->output();

    circa_set_list(out, 0);

    for (int i=0; i < block->length(); i++)
        if (is_statement(block->get(i)))
            circa_set_term(circa_append(out), (caTerm*) block->get(i));
}

void Block__link(VM* vm)
{
    Block* self = (Block*) circa_block(vm->input(0));
    Block* source = (Block*) circa_block(vm->input(1));

    block_link_missing_functions(self, source);
}

void Term__name(VM* vm)
{
    Term* t = as_term_ref(vm->input(0));
    if (t == NULL)
        return vm->throw_str("NULL term");
    if (has_empty_name(t))
        set_string(vm->output(), "");
    else
        copy(term_name(t), vm->output());
}
void Term__to_string(VM* vm)
{
    Term* t = as_term_ref(vm->input(0));
    if (t == NULL)
        return vm->throw_str("NULL term");
    circa::to_string(term_value(t), vm->output());
}
void Term__unique_name(VM* vm)
{
    Term* t = as_term_ref(vm->input(0));
    if (t == NULL)
        return vm->throw_str("NULL term");
    set_value(vm->output(), unique_name(t));
}
void Term__function(VM* vm)
{
    Term* t = as_term_ref(vm->input(0));
    if (t == NULL)
        return vm->throw_str("NULL term");
    set_block(vm->output(), term_function(t));
}
void Term__type(VM* vm)
{
    Term* t = as_term_ref(vm->input(0));
    if (t == NULL)
        return vm->throw_str("NULL term");
    set_type(vm->output(), t->type);
}
void Term__assign(VM* vm)
{
    Term* target = as_term_ref(vm->input(0));
    if (target == NULL) {
        return vm->throw_str("NULL term");
        return;
    }

    Value* source = vm->input(1);

    circa::copy(source, term_value(target));

    // Probably should update term->type at this point.
}

void Term__value(VM* vm)
{
    Term* target = as_term_ref(vm->input(0));
    if (target == NULL)
        return vm->throw_str("NULL term");

    copy(term_value(target), vm->output());
}

void Term__set_value(VM* vm)
{
    Term* target = as_term_ref(vm->input(0));
    if (target == NULL)
        return vm->throw_str("NULL term");

    copy(vm->input(1), term_value(target));
}

void Term__asint(VM* vm)
{
    Term* t = as_term_ref(vm->input(0));
    if (t == NULL)
        return vm->throw_str("NULL term");
    if (!is_int(term_value(t)))
        return vm->throw_str("Not an int");
    set_int(vm->output(), as_int(term_value(t)));
}
void Term__asfloat(VM* vm)
{
    Term* t = as_term_ref(vm->input(0));
    if (t == NULL)
        return vm->throw_str("NULL term");
    
    set_float(vm->output(), to_float(term_value(t)));
}
void Term__id(VM* vm)
{
    Term* t = as_term_ref(vm->input(0));
    set_int(vm->output(), t->id);
}
void Term__index(VM* vm)
{
    Term* t = as_term_ref(vm->input(0));
    if (t == NULL)
        return vm->throw_str("NULL term");
    
    set_int(vm->output(), t->index);
}
void Term__input(VM* vm)
{
    Term* t = as_term_ref(vm->input(0));
    if (t == NULL)
        return vm->throw_str("NULL term");
    int index = vm->input(1)->as_i();
    if (index >= t->numInputs())
        set_term_ref(vm->output(), NULL);
    else
        set_term_ref(vm->output(), t->input(index));
}
void Term__inputs(VM* vm)
{
    Term* t = as_term_ref(vm->input(0));
    if (t == NULL)
        return vm->throw_str("NULL term");

    Value* output = vm->output();
    circa_set_list(output, t->numInputs());

    for (int i=0; i < t->numInputs(); i++)
        set_term_ref(circa_index(output, i), t->input(i));
}
void Term__num_inputs(VM* vm)
{
    Term* t = as_term_ref(vm->input(0));
    if (t == NULL)
        return vm->throw_str("NULL term");
    set_int(vm->output(), t->numInputs());
}
void Term__parent(VM* vm)
{
    Term* t = as_term_ref(vm->input(0));
    if (t == NULL)
        return vm->throw_str("NULL term");
    set_block(vm->output(), t->owningBlock);
}
void Term__contents(VM* vm)
{
    Term* t = as_term_ref(vm->input(0));
    if (t == NULL)
        return vm->throw_str("NULL term");
    set_block(vm->output(), t->nestedContents);
}
void Term__is_input(VM* vm)
{
    Term* t = as_term_ref(vm->input(0));
    set_bool(vm->output(), t != NULL && is_input_placeholder(t));
}
void Term__is_output(VM* vm)
{
    Term* t = as_term_ref(vm->input(0));
    set_bool(vm->output(), t != NULL && is_output_placeholder(t));
}
void Term__is_null(VM* vm)
{
    Term* t = as_term_ref(vm->input(0));
    set_bool(vm->output(), t == NULL);
}
void Term__is_value(VM* vm)
{
    Term* t = as_term_ref(vm->input(0));

    bool consideredValue = is_value(t) || t->function == FUNCS.require;
    set_bool(vm->output(), consideredValue);
}

void Term__source_location(VM* vm)
{
    Term* t = as_term_ref(vm->input(0));
    if (t == NULL)
        return vm->throw_str("NULL reference");

    Value* out = vm->output();
    touch(out);
    set_list(out, 4);
    set_int(list_get(out, 0), t->sourceLoc.col);
    set_int(list_get(out, 1), t->sourceLoc.line);
    set_int(list_get(out, 2), t->sourceLoc.colEnd);
    set_int(list_get(out, 3), t->sourceLoc.lineEnd);
}
void Term__location_string(VM* vm)
{
    Term* t = as_term_ref(vm->input(0));
    if (t == NULL)
        return vm->throw_str("NULL reference");

    get_short_location(t, vm->output());
}
void Term__global_id(VM* vm)
{
    Term* t = as_term_ref(vm->input(0));
    if (t == NULL)
        return vm->throw_str("NULL reference");

    set_int(vm->output(), t->id);
}
void Term__properties(VM* vm)
{
    Term* t = as_term_ref(vm->input(0));
    if (t == NULL)
        return vm->throw_str("NULL reference");
    copy(&t->properties, vm->output());
}
void Term__has_property(VM* vm)
{
    Term* t = as_term_ref(vm->input(0));
    if (t == NULL)
        return vm->throw_str("NULL reference");
    Symbol key = as_symbol(vm->input(1));
    Value* value = term_get_property(t, key);
    set_bool(vm->output(), value != NULL);
}
void Term__property(VM* vm)
{
    Term* t = as_term_ref(vm->input(0));
    if (t == NULL)
        return vm->throw_str("NULL reference");

    Symbol key = as_symbol(vm->input(1));
    Value* value = term_get_property(t, key);

    if (value == NULL)
        set_null(vm->output());
    else
        circa::copy(value, vm->output());
}

void Term__property_opt(VM* vm)
{
    Term* t = as_term_ref(vm->input(0));
    if (t == NULL)
        return vm->throw_str("NULL reference");

    Symbol key = as_symbol(vm->input(1));
    Value* value = term_get_property(t, key);
    Value* defaultValue = vm->input(2);

    if (value == NULL)
        copy(defaultValue, vm->output());
    else
        copy(value, vm->output());
}

void Term__input_property_opt(VM* vm)
{
    Term* t = as_term_ref(vm->input(0));
    if (t == NULL)
        return vm->throw_str("NULL reference");

    int index = as_int(vm->input(1));
    Symbol key = as_symbol(vm->input(2));
    Value* value = term_get_input_property(t, index, key);
    Value* defaultValue = vm->input(3);

    if (value == NULL)
        copy(defaultValue, vm->output());
    else
        copy(value, vm->output());
}

void Term__input_property(VM* vm)
{
    Term* t = as_term_ref(vm->input(0));
    if (t == NULL)
        return vm->throw_str("NULL reference");

    int index = as_int(vm->input(1));
    Symbol key = as_symbol(vm->input(2));
    Value* value = term_get_input_property(t, index, key);
    Value* defaultValue = vm->input(3);

    if (value == NULL)
        copy(defaultValue, vm->output());
    else
        copy(value, vm->output());
}

void Term__has_input_property(VM* vm)
{
    Term* t = as_term_ref(vm->input(0));
    if (t == NULL)
        return vm->throw_str("NULL reference");

    int index = as_int(vm->input(1));
    Symbol key = as_symbol(vm->input(2));

    set_bool(vm->output(), term_get_input_property(t, index, key) == NULL);
}

void Term__trace_dependents(VM* vm)
{
    Term* term = as_term_ref(vm->input(0));
    Block* untilBlock = as_block(vm->input(1));

    Value* out = vm->output();
    set_list(out, 0);
    std::set<Term*> included;

    UpwardIterator upwardIterator(term);
    upwardIterator.stopAt(untilBlock);

    // Look at starting term, because UpwardIterator doesn't yield starting term.
    // TODO: Should fix UpwardIterator
    for (int i=0; i < term->numInputs(); i++) {
        Term* input = term->input(i);
        if (input != NULL) {
            set_term_ref(list_append(out), input);
            included.insert(input);
        }
    }

    for (; upwardIterator.unfinished(); upwardIterator.advance()) {
        Term* current = upwardIterator.current();
        if (current == term || included.find(current) != included.end()) {

            for (int i=0; i < current->numInputs(); i++) {
                Term* input = current->input(i);
                if (input != NULL) {
                    set_term_ref(list_append(out), input);
                    included.insert(input);
                }
            }
        }
    }

    // Order results in the same order as the code.
    list_reverse(out);
}

void reflection_install_functions(NativePatch* patch)
{
    circa_patch_function(patch, "Block.dump", Block__dump);
    circa_patch_function(patch, "Block.find_term", Block__find_term);
    circa_patch_function(patch, "Block.functions", Block__functions);
    circa_patch_function(patch, "Block.statements", Block__statements);
    circa_patch_function(patch, "Block.get_term", Block__get_term);
    circa_patch_function(patch, "Block.id", Block__id);
    circa_patch_function(patch, "Block.input", Block__input);
    circa_patch_function(patch, "Block.inputs", Block__inputs);
    circa_patch_function(patch, "Block.is_null", Block__is_null);
    circa_patch_function(patch, "Block.is_major", Block__is_major);
    circa_patch_function(patch, "Block.link", Block__link);
    circa_patch_function(patch, "Block.list_configs", Block__list_configs);
    circa_patch_function(patch, "Block.output", Block__output);
    circa_patch_function(patch, "Block.outputs", Block__outputs);
    circa_patch_function(patch, "Block.output_placeholder", Block__output_placeholder);
    circa_patch_function(patch, "Block.owner", Block__owner);
    circa_patch_function(patch, "Block.parent", Block__parent);
    circa_patch_function(patch, "Block.property", Block__property);
    circa_patch_function(patch, "Block.properties", Block__properties);
    circa_patch_function(patch, "Block.source_filename", Block__source_filename);
    circa_patch_function(patch, "Block.term_named", Block__term_named);
    circa_patch_function(patch, "Block.terms", Block__terms);
    circa_patch_function(patch, "Block.walk_terms", Block__walk_terms);
    circa_patch_function(patch, "Term.assign", Term__assign);
    circa_patch_function(patch, "Term.asint", Term__asint);
    circa_patch_function(patch, "Term.asfloat", Term__asfloat);
    circa_patch_function(patch, "Term.id", Term__id);
    circa_patch_function(patch, "Term.index", Term__index);
    circa_patch_function(patch, "Term.function", Term__function);
    circa_patch_function(patch, "Term.get_type", Term__type);
    circa_patch_function(patch, "Term.input", Term__input);
    circa_patch_function(patch, "Term.inputs", Term__inputs);
    circa_patch_function(patch, "Term.name", Term__name);
    circa_patch_function(patch, "Term.num_inputs", Term__num_inputs);
    circa_patch_function(patch, "Term.parent", Term__parent);
    circa_patch_function(patch, "Term.contents", Term__contents);
    circa_patch_function(patch, "Term.is_input", Term__is_input);
    circa_patch_function(patch, "Term.is_output", Term__is_output);
    circa_patch_function(patch, "Term.is_null", Term__is_null);
    circa_patch_function(patch, "Term.is_value", Term__is_value);
    circa_patch_function(patch, "Term.source_location", Term__source_location);
    circa_patch_function(patch, "Term.location_string", Term__location_string);
    circa_patch_function(patch, "Term.global_id", Term__global_id);
    circa_patch_function(patch, "Term.to_string", Term__to_string);
    circa_patch_function(patch, "Term.unique_name", Term__unique_name);
    circa_patch_function(patch, "Term.properties", Term__properties);
    circa_patch_function(patch, "Term.has_property", Term__has_property);
    circa_patch_function(patch, "Term.property", Term__property);
    circa_patch_function(patch, "Term.property_opt", Term__property_opt);
    circa_patch_function(patch, "Term.has_input_property", Term__has_input_property);
    circa_patch_function(patch, "Term.input_property", Term__input_property);
    circa_patch_function(patch, "Term.input_property_opt", Term__input_property_opt);
    circa_patch_function(patch, "Term.trace_dependents", Term__trace_dependents);
    circa_patch_function(patch, "Term.value", Term__value);
    circa_patch_function(patch, "Term.set_value", Term__set_value);
}

} // namespace circa
