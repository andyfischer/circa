// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "code_iterators.h"
#include "control_flow.h"
#include "building.h"
#include "function.h"
#include "kernel.h"
#include "inspection.h"
#include "list.h"
#include "loops.h"
#include "names.h"
#include "parser.h"
#include "selector.h"
#include "string_type.h"
#include "switch.h"
#include "symbols.h"
#include "term.h"
#include "type.h"

namespace circa {

void on_term_created(Term* term)
{
    // debugging hook
}

Term* apply(Block* block, Term* function, TermList const& inputs, Value* name)
{
    ca_assert(function != NULL);

    block_start_changes(block);

    // Figure out the term position; it should be placed before any output() terms.
    // (unless it's an output term itself).
    int position = block->length();
    if (function != FUNCS.output) {
        while (position > 0) {
            Term* preceding = block->get(position - 1);
            if (preceding == NULL)
                break;

            // Position before output() terms.
            if (preceding->function == FUNCS.output) {
                position--;
                continue;
            }

            break;
        }
    }

    // Create the term
    Term* term = block->appendNew();
    stat_increment(TermCreated);

    on_term_created(term);

    // Position the term before any output_placeholder terms.
    block->move(term, position);

    if (name != NULL && !is_null(name) && !string_equals(name, ""))
        rename(term, name);

    for (int i=0; i < inputs.length(); i++)
        set_input(term, i, inputs[i]);

    // update term->function, change_function will also update the declared type.
    change_function(term, function);

    update_unique_name(term);

    // Post-compile steps

    // Possibly run the function's postCompile handler
    if (is_function(function)) {
        PostCompileFunc func = nested_contents(function)->overrides.postCompile;

        if (func != NULL)
            func(term);
    }

    return term;
}

Term* apply(Block* block, Term* function, TermList const& inputs, const char* nameStr)
{
    Value name;
    if (nameStr != NULL)
        set_string(&name, nameStr);
    return apply(block, function, inputs, &name);
}

Term* apply_dynamic_method(Block* block, Symbol methodName, TermList const& inputs, Value* name)
{
    Term* term = apply(block, FUNCS.dynamic_method, inputs, name);
    term->setStringProp(s_method_name, symbol_as_string(methodName));
    return term;
}

Term* apply_spec(Block* block, Value* spec)
{
    Term* function = as_term_ref(list_get(spec, 0));
    Value* inputs = list_get(spec, 1);

    TermList inputList;
    for (int i=0; i < list_length(inputs); i++) {
        inputList.append(as_term_ref(list_get(inputs, i)));
    }

    Term* result = apply(block, function, inputList, "");

    for (int i=2; i < list_length(spec); i++) {
        Value* key = list_get(spec, i);
        if (symbol_eq(key, s_Name)) {
            i++;
            rename(result, list_get(spec, i));
        } else {
            i++;
            term_set_property(result, as_symbol(key), list_get(spec, i));
        }
    }

    return result;
}

void set_input(Term* term, int index, Term* input)
{
    block_start_changes(term->owningBlock);

    Term* previousInput = NULL;
    if (index < term->numInputs())
        previousInput = term->input(index);

    while (index >= term->numInputs())
        term->inputs.push_back(NULL);

    term->inputs[index].term = input;
}

void set_inputs(Term* term, TermList const& inputs)
{
    Term::InputList previousInputs = term->inputs;

    term->inputs.resize(inputs.length());
    for (int i=0; i < inputs.length(); i++)
        term->inputs[i] = Term::Input(inputs[i]);
}

void insert_input(Term* term, Term* input)
{
    term->inputs.insert(term->inputs.begin(), Term::Input(NULL));
    set_input(term, 0, input);
}

void insert_input(Term* term, int index, Term* input)
{
    term->inputs.insert(term->inputs.begin() + index, Term::Input(NULL));
    set_input(term, index, input);
}

void change_function(Term* term, Term* function)
{
    if (term->function == function)
        return;

    Term* previousFunction = term->function;

    term->function = function;

    respecialize_type(term);

    if (function != NULL
            && is_function(function) 
            && nested_contents(function)->functionAttrs.hasNestedContents)
        make_nested_contents(term);
}

void set_declared_type(Term *term, Type *newType)
{
    if (newType == NULL)
        newType = TYPES.any;

    // Don't allow the nil type to be used as a declared type
    if (newType == TYPES.nil)
        newType = TYPES.any;

    ca_assert(term != NULL);
    ca_assert(newType != NULL);

    if (term->type == newType)
        return;

    if (newType != NULL)
        type_incref(newType);
    if (term->type != NULL)
        type_decref(term->type);

    term->type = newType;

    set_null(term_value(term));
}

void respecialize_type(Term* term)
{
    Type* outputType = derive_specialized_output_type(term->function, term);
    if (outputType != term->type)
        set_declared_type(term, outputType);
}

void rename(Term* termToRename, Value* name)
{
    // No-op if term already has this name.
    if (equals(&termToRename->nameValue, name))
        return;

    Block* block = termToRename->owningBlock;

    // Update binding in the owning block.
    if (block != NULL) {
        if (!has_empty_name(termToRename)) {
            termToRename->owningBlock->names.remove(termToRename->name());
            set_null(&termToRename->nameValue);
        }
        termToRename->owningBlock->bindName(termToRename, name);
    }

    copy(name, &termToRename->nameValue);
    update_unique_name(termToRename);

    // Update unique ordinal. If any neighbor term has the same name, then give this
    // term a greater ordinal value.
    termToRename->uniqueOrdinal = 0;

    if (block != NULL) {
        for (int i=0; i < block->length(); i++) {
            Term* neighbor = block->get(i);
            if (neighbor == termToRename)
                continue;
            if (neighbor == NULL)
                continue;

            if (equals(&neighbor->nameValue, name)) {
                // Check if the neighbor has ordinal value 0 (meaning no name collision).
                // If so, then promote it to 1 (meaning there is a collision.
                if (neighbor->uniqueOrdinal == 0)
                    neighbor->uniqueOrdinal = 1;

                if (neighbor->uniqueOrdinal >= termToRename->uniqueOrdinal)
                    termToRename->uniqueOrdinal = neighbor->uniqueOrdinal + 1;
            }
        }
    }

    // Handle change cascades.

    // Possibly store the shadowed name binding (this is only computed if needed)
    bool foundShadowedNameBinding = false;
    Term* shadowedNameBinding = NULL;
    
    if (name != NULL) {
        // The new name may have shadowed an existing name.
        for (NameVisibleIterator it(termToRename); it.unfinished(); ++it) {
            Term* possibleUser = it.current();
            for (int i=0; i < possibleUser->numInputs(); i++) {

                // Only look at inputs that have our name binding.
                if (possibleUser->input(i) == NULL)
                    continue;
                if (!equals(&possibleUser->input(i)->nameValue, name))
                    continue;

                if (!foundShadowedNameBinding) {
                    foundShadowedNameBinding = true;
                    Value termVal;
                    termVal.set_term(termToRename);
                    shadowedNameBinding = find_name_at(&termVal, name);
                    // shadowedNameBinding might still be NULL.
                }

                // We found a term that is using the name that we just rebound,
                // and the term is at a location where our new name should be visible.
                // So, this term is a candidate for a rebinding from shadowing.
                //
                // We're going to be conservative, and only touch bindings that were
                // bound to the previous name binding at this location. There probably
                // shouldn't be any other kind of binding, but we're still refining
                // things.
                
                if (possibleUser->input(i) == shadowedNameBinding) {
                    remap_pointers_quick(possibleUser, possibleUser->input(i), termToRename);
                }
            }
        }
    }
}

void rename(Term* term, const char* name)
{
    Value nameVal;
    set_string(&nameVal, name);
    return rename(term, &nameVal);
}

Term* create_value(Block* block, Type* type, const char* name)
{
    // This function is safe to call while bootstrapping.
    ca_assert(type != NULL);

    Term* term = apply(block, FUNCS.value, TermList(), name);

    set_declared_type(term, type);
    make(type, term_value(term));

    if (type == TYPES.type) {
        if (name != NULL)
            set_string(&as_type(term)->name, name);
        as_type(term)->declaringTerm = term;
    }

    return term;
}

Term* create_value(Block* block, const char* typeName, const char* name)
{
    Term* type = NULL;

    type = find_name(block, typeName);

    if (type == NULL)
        internal_error(std::string("Couldn't find type: ")+typeName);

    return create_value(block, as_type(term_value(type)), name);
}

Term* create_value(Block* block, Value* initialValue, const char* name)
{
    Term* term = create_value(block, initialValue->value_type, name);
    copy(initialValue, term_value(term));
    return term;
}

Term* create_string(Block* block, const char* s, const char* name)
{
    Term* term = create_value(block, TYPES.string, name);
    set_string(term_value(term), s);
    return term;
}

Term* create_int(Block* block, int i, const char* name)
{
    Term* term = create_value(block, TYPES.int_type, name);
    set_int(term_value(term), i);
    return term;
}

Term* create_float(Block* block, float f, const char* name)
{
    Term* term = create_value(block, TYPES.float_type, name);
    set_float(term_value(term), f);
    return term;
}

Term* create_bool(Block* block, bool b, const char* name)
{
    Term* term = create_value(block, TYPES.bool_type, name);
    set_bool(term_value(term), b);
    return term;
}

Term* create_void(Block* block, const char* name)
{
    return create_value(block, TYPES.void_type, name);
}

Term* create_list(Block* block, const char* name)
{
    Term* term = create_value(block, TYPES.list, name);
    return term;
}

Block* create_block(Block* owner, const char* name)
{
    return nested_contents(apply(owner, FUNCS.section_block, TermList(), name));
}

Term* create_type(Block* block, const char* name)
{
    return create_value(block, TYPES.type, name);
}

Term* create_type_value(Block* block, Type* value, const char* name)
{
    Term* term = create_value(block, TYPES.type, name);
    set_type(term_value(term), value);

    if (value->declaringTerm == NULL)
        value->declaringTerm = term;
        
    return term;
}

Term* create_symbol_value(Block* block, int value, const char* name)
{
    Term* term = create_value(block, TYPES.symbol, name);
    set_symbol(term_value(term), value);
    return term;
}

Term* append_input_placeholder(Block* block)
{
    int count = count_input_placeholders(block);
    Term* term = apply(block, FUNCS.input, TermList());
    block->move(term, count);
    return term;
}
Term* append_output_placeholder(Block* block, Term* result)
{
    int count = count_output_placeholders(block);
    Term* term = apply(block, FUNCS.output, TermList(result));
    block->move(term, block->length() - count - 1);
    return term;
}
Term* append_output_placeholder_with_description(Block* block, Value* description)
{
    if (is_string(description)) {
        Term* result = append_output_placeholder(block, find_name(block, description));
        rename(result, description);
        return result;
    }

    Value* descriptionTag = list_get(description, 0);

    if (as_symbol(descriptionTag) == s_Name) {
        Value* name = list_get(description, 1);
        Term* result = append_output_placeholder(block, find_name(block, name));
        rename(result, name);
        return result;
    } else if (as_symbol(descriptionTag) == s_Control) {
        Term* result = append_output_placeholder(block, NULL);
        result->setBoolProp(s_Control, true);
        return result;
    } else {
        Term* result = append_output_placeholder(block, NULL);
        return result;
    }
}
Term* prepend_output_placeholder(Block* block, Term* result)
{
    return apply(block, FUNCS.output, TermList(result));
}

Term* insert_output_placeholder(Block* block, Term* result, int location)
{
    Term* term = apply(block, FUNCS.output, TermList(result));
    if (location != 0) {
        block->move(term, block->length() - location - 1);
    }
    return term;
}

void get_input_description(Term* input, Value* result)
{
    // Primary input.
    if (input_placeholder_index(input) == 0) {
        // return :Primary
        set_symbol(result, s_Primary);
        return;
    }

    // return :Anonymous
    set_symbol(result, s_Anonymous);
}

Term* find_output_placeholder_with_name(Block* block, Value* name)
{
    for (int i=0;; i++) {
        Term* placeholder = get_output_placeholder(block, i);
        if (placeholder == NULL)
            return NULL;
        if (equals(&placeholder->nameValue, name))
            return placeholder;
    }
}

Term* find_output_from_description(Block* block, Value* description)
{
    if (is_string(description))
        return find_output_placeholder_with_name(block, description);

    Value* tag = list_get(description, 0);
    if (as_symbol(tag) == s_Name) {
        return find_output_placeholder_with_name(block, list_get(description, 1));
    }

    else if (as_symbol(tag) == s_Control) {
        for (int i=0;; i++) {
            Term* placeholder = get_output_placeholder(block, i);
            if (placeholder == NULL)
                return NULL;
            if (placeholder->hasProperty(s_Control))
                return placeholder;
        }
    }
    else if (as_symbol(tag) == s_ExtraReturn) {
        for (int i=0;; i++) {
            Term* placeholder = get_output_placeholder(block, i);
            if (placeholder == NULL)
                return NULL;
            if (placeholder->hasProperty(s_ExtraReturn))
                return placeholder;
        }
    }

    return NULL;
}

void get_output_description(Term* output, Value* result)
{
    // control output
    if (output->hasProperty(s_Control)) {
        // return [:control]
        set_list(result, 1);
        set_symbol(list_get(result, 0), s_Control);
        return;
    }

    // extraReturn output
    else if (output->hasProperty(s_ExtraReturn)) {
        // return [:extraReturn <index>]
        set_list(result, 2);
        set_symbol(list_get(result, 0), s_ExtraReturn);
        set_int(list_get(result, 1), output->intProp(s_ExtraReturn, 0));
        return;
    }

    // Named output.
    else if (!has_empty_name(output)) {
        // return [:name, <name>]
        set_list(result, 2);
        set_symbol(list_get(result, 0), s_Name);
        copy(term_name(output), list_get(result, 1));
        return;
    }

    // Primary output.
    else if (input_placeholder_index(output) == 0) {
        // return [:primary]
        set_list(result, 1);
        set_symbol(list_get(result, 0), s_Primary);
        return;
    }

    set_list(result, 1);
    set_symbol(list_get(result, 0), s_Anonymous);
}

Term* get_output_term(Term* term, int index)
{
    if (index == 0)
        return term;
    else
        return get_extra_output(term, index - 1);
}

Term* find_or_create_output_term(Term* term, int index)
{
    if (index == 0)
        return term;

    Block* block = term->owningBlock;

    Term* result = NULL;
    int extraIndex = index - 1;
    for (int i=0; i <= extraIndex; i++) {
        Term* output = block->getSafe(term->index + 1 + i);

        if (output == NULL || output->function != FUNCS.extra_output) {
            // Insert a new extra_output call.
            output = apply(block, FUNCS.extra_output, TermList(term));
            move_to_index(output, term->index + 1 + i);
        }

        if (i == extraIndex)
            result = output;
    }

    return result;
}

Term* get_extra_output(Term* term, int index)
{
    Term* position = term->owningBlock->getSafe(term->index + index + 1);
    if (position != NULL && position->function == FUNCS.extra_output)
        return position;
    return NULL;
}

int count_outputs(Term* term)
{
    for (int i=0;; i++)
        if (get_output_term(term, i) == NULL)
            return i;
}

Term* find_intermediate_result_for_output(Term* location, Term* output)
{
    Value description;
    get_output_description(output, &description);
    Value* descriptionTag = list_get(&description, 0);

    // Control value
    if (as_symbol(descriptionTag) == s_Control) {
        Block* block = location->owningBlock;
        for (int i = location->index; i >= 0; i--) {
            Term* term = block->get(i);
            if (term == NULL)
                continue;
            if (term->boolProp(s_Control, false))
                return term;
        }
        return NULL;
    }
    
    // Check whether the output's connection is valid at this location
    Term* result = output->input(0);
    if (result != NULL
            && result->owningBlock == location->owningBlock
            && result->index < location->index)
        return result;

    // Nearest with same name
    if (!has_empty_name(output))
        return find_name_at(location, output->name());

    return NULL;
}

void update_extra_output_count(Term* term, int count)
{
    Block* block = term->owningBlock;

    for (int i=0; i < count; i++) {

        int expectedIndex = term->index + i + 1;

        Term* existing = block->getSafe(expectedIndex);
        if (existing != NULL && existing->function != FUNCS.extra_output)
            existing = NULL;
        
        if (existing == NULL) {
            Term* output = apply(block, FUNCS.extra_output, TermList(term));
            move_to_index(output, expectedIndex);
        }
    }
}

void update_extra_outputs(Term* term, Block* targetBlock)
{
    int outputCount = count_output_placeholders(targetBlock);
    update_extra_output_count(term, outputCount - 1);

    for (int index=1; ; index++) {
        Term* placeholder = get_output_placeholder(targetBlock, index);
        if (placeholder == NULL)
            break;

        Term* extraOutput = get_output_term(term, index);
        rename(extraOutput, &placeholder->nameValue);
        set_declared_type(extraOutput, placeholder->type);
    }
}

Symbol block_has_state(Block* block)
{
    if (find_enclosing_major_block(block) == FUNCS.declared_state->nestedContents)
        return s_no;

    if (block_has_property(block, s_has_state))
        return block_get_symbol_prop(block, s_has_state, s_no);

    // Temporarily set to :Maybe in case of recursion.
    block_set_symbol_prop(block, s_has_state, s_maybe);

    Symbol result = s_no;

    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);
        if (is_declared_state(term)) {
            result = s_yes;
            break;
        }

        if (uses_dynamic_dispatch(term) && result == s_no)
            result = s_maybe;

        Block* contents = static_dispatch_block(term);
        if (contents == NULL)
            continue;

        Symbol hasState = block_has_state(contents);
        if (hasState == s_yes) {
            result = s_yes;
            break;
        }

        if (hasState == s_maybe)
            result = s_maybe;
    }

    block_set_symbol_prop(block, s_has_state, result);
    return result;
}

void block_start_changes(Block* block)
{
    if (block->inProgress)
        return;

    block->inProgress = true;

    // For any minor block that is inProgress, make sure the parent block is also
    // marked inProgress.
    
    if (is_minor_block(block))
        block_start_changes(get_parent_block(block));
}

void block_finish_changes(Block* block)
{
    if (!block->inProgress)
        return;

    // Perform cleanup

    // Remove NULLs
    block->removeNulls();

    // Make sure nested minor blocks are finished.
    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);

        if (term->nestedContents != NULL && is_minor_block(term->nestedContents))
            block_finish_changes(term->nestedContents);
    }

    if (is_major_block(block))
        update_term_user_lists(block);

    fix_forward_function_references(block);
    annotate_stateful_values(block);

    // After we are finished creating outputs, update any nested control flow operators.
    update_for_control_flow(block);

    // Make sure the primary output is connected.
    if (is_minor_block(block)) {
        Term* output = get_output_placeholder(block, 0);

        // Don't mess with the primary if-block output.
        if (output != NULL && output->input(0) == NULL && !is_switch_block(output->owningBlock)) {
            set_input(output, 0, find_expression_for_implicit_output(block));
            respecialize_type(output);
        }
    }

    // Update :HasDynamicDispatch
    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);
        if (uses_dynamic_dispatch(term))
            block_set_bool_prop(block, s_HasDynamicDispatch, true);
    }

    // Propogate annotate() and annotate_block() values
    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);
        if (term->function == FUNCS.annotate && term->numInputs() >= 2) {
            Value* key = term_value(term->input(1));
            if (is_symbol(key)) {
                if (term->input(2) == NULL)
                    term->input(0)->setBoolProp(as_symbol(key), true);
                else
                    term->input(0)->setProp(as_symbol(key), term_value(term->input(2)));
            }
        }
        
        if (term->function == FUNCS.annotate_block && term->numInputs() >= 1) {
            Value* key = term_value(term->input(0));
            if (is_symbol(key)) {
                if (term->input(1) == NULL)
                    set_bool(block_insert_property(block, as_symbol(key)), true);
                else
                    copy(term_value(term->input(1)), block_insert_property(block, as_symbol(key)));
            }
        }
    }

    block->inProgress = false;
}

void update_term_user_lists(Block* block)
{
    for (BlockIterator it(block); it; ++it) {
        Term* term = *it;
        term->users.clear();

        if (term->function == FUNCS.extra_output)
            continue;

        for (int depi=0; depi < term->numDependencies(); depi++) {
            Term* dep = term->dependency(depi);

            if (dep == NULL)
                continue;
                
            //if (!is_under_same_major_block(term, dep))
            //    continue;

            dep->users.append(term);
        }
    }
}

void annotate_stateful_values(Block* block)
{
    for (BlockIteratorFlat it(block); it; ++it) {
        Term* term = *it;
        if (term->function != FUNCS.declared_state)
            continue;

        Term* result = find_name(block, term_name(term)); 
        term_set_bool_prop(result, s_LocalStateResult, true);

        BlockIterator2 exitPointSearch;
        exitPointSearch.startAt(term);

        for (; exitPointSearch; ++exitPointSearch) {
            if (!is_exit_point(*exitPointSearch))
                continue;

            Value termVal;
            termVal.set_term(*exitPointSearch);

            Term* localResult = find_name_at(&termVal, term_name(term)); 
            term_set_bool_prop(localResult, s_LocalStateResult, true);
        }
    }
}

Term* apply_before(Term* existing, Term* function, int input)
{
    Block* block = existing->owningBlock;
    Term* newTerm = apply(block, function, TermList(existing->input(input)));
    block->move(newTerm, existing->index);
    set_input(existing, input, newTerm);
    return newTerm;
}
Term* apply_after(Term* existing, Term* function)
{
    Block* block = existing->owningBlock;

    Term* newTerm = apply(block, function, TermList(existing));
    block->move(newTerm, existing->index + 1);

    return newTerm;
}
void move_before(Term* movee, Term* position)
{
    ca_assert(movee->owningBlock == position->owningBlock);
    movee->owningBlock->move(movee, position->index);
}

void move_after(Term* movee, Term* position)
{
    Block* block = movee->owningBlock;
    int pos = position->index + 1;

    // Make sure the position is after any extra_output() terms
    while (pos < block->length()
            && block->get(pos) != NULL
            && block->get(pos)->function == FUNCS.extra_output)
        pos++;

    // If 'movee' is currently before 'position', then the desired index is one less
    if (movee->index < position->index)
        pos--;

    block->move(movee, pos);
}

void move_after_inputs(Term* term)
{
    Block* block = term->owningBlock;
    int inputCount = count_input_placeholders(block);
    block->move(term, inputCount);
}

bool term_belongs_at_block_end(Term* term)
{
    if (term == NULL)
        return false;

    if (term->function == FUNCS.output)
        return true;

    if (term->boolProp(s_Final, false))
        return true;

    return false;
}

void move_before_outputs(Term* term)
{
    Block* block = term->owningBlock;

    // Walk backwards to find the target position
    int position = block->length();
    for (; position > 0; position--) {
        Term* preceding = block->get(position - 1);

        if (is_output_placeholder(preceding))
            continue;

        break;
    }

    // We now have the position of the 1st final term. If this term isn't
    // an output term itself, then move the position back one more.
    if (!is_output_placeholder(term))
        position--;

    block->move(term, position);
}

void move_before_final_terms(Term* term)
{
    Block* block = term->owningBlock;

    // Walk backwards to find the target position
    int position = block->length();
    for (; position > 0; position--) {
        Term* preceding = block->get(position - 1);

        if (term_belongs_at_block_end(preceding))
            continue;

        break;
    }

    // We now have the position of the 1st final term. If this term isn't
    // a final term itself, then move the position back one more.
    if (!term_belongs_at_block_end(term))
        position--;

    block->move(term, position);
}

void move_to_index(Term* term, int index)
{
    term->owningBlock->move(term, index);
}

void input_placeholders_to_list(Block* block, TermList* list)
{
    for (int i=0;; i++) {
        Term* placeholder = get_input_placeholder(block, i);
        if (placeholder == NULL)
            break;
        list->append(placeholder);
    }
}

void list_outer_pointers(Block* block, TermList* list)
{
    for (BlockInputIterator it(block); it.unfinished(); it.advance()) {
        if (it.currentInput()->owningBlock != block)
            list->appendUnique(it.currentInput());
    }
}

int find_input_index_for_pointer(Term* call, Term* input)
{
    for (int i=0; i < call->numInputs(); i++) {
        if (call->input(i) == input)
            return i;
    }
    return -1;
}

void check_to_add_primary_output_placeholder(Block* block)
{
    Term* output = get_output_placeholder(block, 0);

    if (output == NULL)
        prepend_output_placeholder(block, find_expression_for_implicit_output(block));
}

void update_declared_type(Term* term)
{
    Block* function = nested_contents(term->function);

    Type* outputType = get_output_type(function, 0);

    if (function->overrides.specializeType != NULL)
        outputType = function->overrides.specializeType(term);

    if (outputType == NULL)
        outputType = TYPES.any;

    set_declared_type(term, outputType);
}

void rewrite(Term* term, Term* function, TermList const& inputs)
{
    change_function(term, function);
    for (int i=0; i < inputs.length(); i++)
        set_input(term, i, inputs[i]);

    update_declared_type(term);
}

void remove_term(Term* term)
{
    int index = term->index;
    Block* block = term->owningBlock;

    erase_term(term);

    for (int i=index; i < block->_terms.length()-1; i++) {
        block->_terms.setAt(i, block->_terms[i+1]);
        if (block->_terms[i] != NULL)
            block->_terms[i]->index = i;
    }
    block->_terms.resize(block->_terms.length()-1);
}

void remap_pointers_quick(Term* term, Term* old, Term* newTerm)
{
    for (int i=0; i < term->numDependencies(); i++)
        if (term->dependency(i) == old)
            term->setDependency(i, newTerm);
}

void remap_pointers_quick(Block* block, Term* old, Term* newTerm)
{
    for (int i=0; i < block->length(); i++)
        remap_pointers_quick(block->get(i), old, newTerm);
}

bool term_is_nested_in_block(Term* term, Block* block)
{
    while (term != NULL) {
        if (term->owningBlock == block)
            return true;

        term = parent_term(term);
    }

    return false;
}

void create_inputs_for_outer_references(Term* term)
{
    Block* block = nested_contents(term);
    TermMap outerToInnerMap;

    for (BlockIterator it(block); it; ++it) {
        Term* innerTerm = *it;
        for (int inputIndex=0; inputIndex < innerTerm->numInputs(); inputIndex++) {
            Term* input = innerTerm->input(inputIndex);
            if (input == NULL)
                continue;

            if (!term_is_nested_in_block(input, block)) {
                // This is an outer reference

                // Check if we've already created a placeholder for this one
                Term* existingPlaceholder = outerToInnerMap[input];

                if (existingPlaceholder != NULL) {
                    remap_pointers_quick(innerTerm, input, existingPlaceholder);

                } else {
                    // Need to create a new placeholder
                    int placeholderIndex = term->numInputs();
                    Term* placeholder = apply(block, FUNCS.input, TermList(), &input->nameValue);
                    set_declared_type(placeholder, input->type);
                    block->move(placeholder, placeholderIndex);
                    set_input(term, placeholderIndex, placeholder);
                    remap_pointers_quick(innerTerm, input, placeholder);
                }
            }
        }
    }
}

Term* preceding_term(Term* term)
{
    return term->owningBlock->getSafe(term->index - 1);
}

Term* preceding_term_recr_minor(Term* term)
{
    Block* block = term->owningBlock;
    int position = term->index - 1;

    while (true) {
        if (position < 0) {
            Block* parent = get_parent_block(block);
            if (parent != NULL && is_minor_block(block)) {
                position = parent_term(block)->index - 1;
                block = parent;
                continue;
            } else {
                return NULL;
            }
        }

        if (block->get(position) == NULL)
            continue;

        return block->get(position);
    }
}

Term* following_term(Term* term)
{
    return term->owningBlock->getSafe(term->index + 1);
}

void fix_forward_function_references(Block* block)
{
    for (BlockIterator it(block); it; ++it) {
        Term* term = *it;
        if (term->function == NULL || term->function == FUNCS.unknown_function) {
            // See if we can now find this function
            Value* functionName = term->getProp(s_Syntax_FunctionName);

            if (functionName != NULL) {
                Term* func = find_name(block, functionName, s_LookupFunction);
                if (func != NULL)
                    change_function(term, func);
            }
        }
    }
}

} // namespace circa
