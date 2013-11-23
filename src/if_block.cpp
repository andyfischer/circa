// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <set>

#include "common_headers.h"

#include "block.h"
#include "code_iterators.h"
#include "kernel.h"
#include "building.h"
#include "interpreter.h"
#include "inspection.h"
#include "term.h"
#include "type.h"
#include "type_inference.h"
#include "update_cascades.h"

#include "if_block.h"

namespace circa {

static Term* if_block_add_input(Term* ifCall, Term* input);

struct CaseIterator
{
    BlockIteratorFlat blockIterator;

    CaseIterator(Block* block)
      : blockIterator(block)
    {
        advanceWhileInvalid();
    }
     
    bool finished()
    {
        return blockIterator.finished();
    }
    void advance()
    {
        blockIterator.index++;
        advanceWhileInvalid();
    }
    void advanceWhileInvalid()
    {
    possibly_invalid:
        if (finished())
            return;

        if (blockIterator.current()->function != FUNCS.case_func) {
            blockIterator.advance();
            goto possibly_invalid;
        }
    }

    Term* current()
    {
        ca_assert(blockIterator.current()->function == FUNCS.case_func);
        return blockIterator.current();
    }
    int index()
    {
        return blockIterator.index;
    }

    bool unfinished() { return !finished(); }
    void operator++() { advance(); }
};

int if_block_count_cases(Block* block)
{
    int result = 0;
    for (int i=0; i < block->length(); i++)
        if (block->get(i) != NULL && block->get(i)->function == FUNCS.case_func)
            result++;
    return result;
}

int case_block_get_index(Block* caseBlock)
{
    Block* if_block = get_block_for_case_block(caseBlock);

    int index = 0;
    for (int i=0; i < if_block->length(); i++) {
        if (if_block->get(i)->nestedContents == caseBlock)
            return index;

        if (if_block->get(i)->function == FUNCS.case_func)
            index++;
    }

    internal_error("case not found in case_block_get_index");
    return -1;
}

Term* case_find_condition_check(Block* block)
{
    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);
        if (term->function == FUNCS.case_condition_bool)
            return term;
    }
    return NULL;
}

Term* case_find_condition(Block* block)
{
    Term* conditionCheck = case_find_condition_check(block);
    if (conditionCheck != NULL)
        return conditionCheck->input(0);
    return NULL;
}

Term* if_block_add_input(Term* ifBlock, Term* input)
{
    Block* contents = nested_contents(ifBlock);

    int existingInputCount = ifBlock->numInputs();

    Term* placeholder = append_input_placeholder(contents);
    rename(placeholder, term_name(input));
    change_declared_type(placeholder, input->type);

    set_input(ifBlock, existingInputCount, input);

    // Add a corresponding input placeholder to each case
    for (CaseIterator it(contents); it.unfinished(); it.advance()) {
        Term* term = it.current();
        Block* caseContents = nested_contents(term);
        Term* casePlaceholder = append_input_placeholder(caseContents);
        change_declared_type(casePlaceholder, placeholder->type);
        rename(casePlaceholder, term_name(input));
    }

    return placeholder;
}

Term* if_block_prepend_primary_output(Term* ifBlock)
{
    Block* contents = nested_contents(ifBlock);

    Term* placeholder = prepend_output_placeholder(contents, NULL);

    // Insert a corresponding output in each case.
    for (CaseIterator it(contents); it.unfinished(); it.advance()) {
        Block* caseContents = nested_contents(it.current());

        Term* result = find_expression_for_implicit_output(caseContents);

        // If the last term already has a name then don't make it the default output.
        if (result != NULL && !has_empty_name(result))
            result = NULL;

        prepend_output_placeholder(nested_contents(it.current()), result);
    }
    return placeholder;
}

Term* if_block_append_output(Block* block, caValue* description)
{
    // Check if such an output already exists.
    Term* existing = find_output_from_description(block, description);
    if (existing != NULL)
        return existing;

    Term* blockPlaceholder = NULL;

    blockPlaceholder = append_output_placeholder_with_description(block, description);

    // Add a corresponding output placeholder to each case
    for (CaseIterator it(block); it.unfinished(); it.advance()) {

        Block* caseContents = nested_contents(it.current());

        append_output_placeholder_with_description(caseContents, description);
    }

    return blockPlaceholder;
}

Block* if_block_get_case(Block* block, int index)
{
    for (int i=0; i < block->length(); i++) {
        if (block->get(i) == NULL || block->get(i)->function != FUNCS.case_func)
            continue;

        if (index == 0)
            return nested_contents(block->get(i));

        index--;
    }
    return NULL;
}

void if_block_start(Block* block)
{
    // Create a placeholder for primary output
    append_output_placeholder(block, NULL);
}

static void if_block_setup_new_case(Block* block, Term* newCase)
{
    // Add existing input placeholders to this case
    for (int i=0;; i++) {
        Term* placeholder = get_input_placeholder(block, i);
        if (placeholder == NULL) break;
        Term* localPlaceholder = append_input_placeholder(nested_contents(newCase));
        change_declared_type(localPlaceholder, placeholder->type);
        rename(localPlaceholder, term_name(placeholder));
    }

    // Add existing output placeholders to this case
    for (int i=0;; i++) {
        Term* placeholder = get_output_placeholder(block, i);
        if (placeholder == NULL) break;
        Term* localPlaceholder = append_output_placeholder(nested_contents(newCase), NULL);
        change_declared_type(localPlaceholder, placeholder->type);
        rename(localPlaceholder, term_name(placeholder));
    }
}

Term* if_block_append_case(Block* block)
{
    int insertPos = 0;
    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);

        if (term->function == FUNCS.input)
            insertPos = term->index + 1;

        // Insert position is right after the last case.
        if (term->function == FUNCS.case_func && term->name != "else")
            insertPos = term->index + 1;
    }

    Term* newCase = apply(block, FUNCS.case_func, TermList());
    block->move(newCase, insertPos);

    if_block_setup_new_case(block, newCase);
    return newCase;
}

void case_add_condition_check(Block* caseBlock, Term* condition)
{
    apply(caseBlock, FUNCS.case_condition_bool, TermList(condition));
}

bool is_case_block(Block* block)
{
    return block->owningTerm != NULL && block->owningTerm->function == FUNCS.case_func;
}
bool is_if_block(Block* block)
{
    return block->owningTerm != NULL && block->owningTerm->function == FUNCS.if_block;
}
Block* get_block_for_case_block(Block* block)
{
    return get_parent_block(block);
}
Term* if_block_get_output_by_name(Block* block, const char* name)
{
    for (int i=0;; i++) {
        Term* term = get_output_placeholder(block, i);
        if (term == NULL)
            break;
        if (term->name == name)
            return term;
    }
    return NULL;
}

void if_block_finish_appended_case(Block* block, Term* caseTerm)
{
    // Add an output placeholder
    apply(nested_contents(caseTerm), FUNCS.output,
        TermList(find_expression_for_implicit_output(nested_contents(caseTerm))));
}

void if_block_create_input_placeholders_for_outer_pointers(Term* ifCall)
{
    Block* contents = nested_contents(ifCall);
    TermList outerTerms;

    // Find outer pointers across each case
    for (CaseIterator it(contents); it.unfinished(); it.advance()) {
        list_outer_pointers(nested_contents(it.current()), &outerTerms);
    }

    ca_assert(ifCall->numInputs() == 0);

    // Create input placeholders and add inputs for all outer pointers
    for (int i=0; i < outerTerms.length(); i++) {
        Term* outer = outerTerms[i];

        set_input(ifCall, i, outer);
        Term* placeholder = append_input_placeholder(nested_contents(ifCall));
        rename(placeholder, term_name(outer));

        // Go through each case and repoint to this new placeholder
        for (CaseIterator it(contents); it.unfinished(); it.advance()) {
            remap_pointers_quick(nested_contents(it.current()), outer, placeholder);
        }
    }
}

void if_block_turn_outer_name_rebinds_into_outputs(Term* ifCall, Block *caseBlock)
{
    Block* mainBlock = nested_contents(ifCall);
    Block* outerBlock = ifCall->owningBlock;

    for (int i=0; i < caseBlock->length(); i++) {
        Term* term = caseBlock->get(i);
        if (term->name == "")
            continue;

        caValue* name = term_name(term);

        Term* outer = find_name(outerBlock, name);
        if (outer == NULL)
            continue;

        // Don't look at names outside the major block.
        if (find_nearest_major_block(outer->owningBlock) !=
                find_nearest_major_block(outerBlock))
            continue;

        // This term rebinds an outer name.

        // First, bring in the outer name as an input to the block.
        
#if 0
        // Check if we already have an output for this name.
        Term* inputPlaceholder = find_input_placeholder_with_name(mainBlock, name);

        // Create it if necessary
        if (inputPlaceholder == NULL) {
            inputPlaceholder = if_block_add_input(ifCall, outer);
            rename(inputPlaceholder, name);

            // Fix the new input placeholders to have the correct name and input.
            for (CaseIterator it(mainBlock); it.unfinished(); it.advance()) {
                Block* caseContents = nested_contents(it.current());
                Term* casePlaceholder = get_input_placeholder(caseContents,
                    inputPlaceholder->index);
                ca_assert(casePlaceholder != NULL);
                rename(casePlaceholder, name);
            }
        }
#endif

        // Now make sure there is an output placeholder for this name.
        Term* outputPlaceholder = find_output_placeholder_with_name(mainBlock, name);

        if (outputPlaceholder == NULL)
            outputPlaceholder = if_block_append_output(mainBlock, name);
    }
}

void write_all_names_to_list(Block* block, List* names)
{
    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);
        if (term->name != "")
            set_string(names->append(), term->name);
    }
}

void if_block_turn_common_rebinds_into_outputs(Term* ifCall)
{
    // Find names which are bound in every block (and not already outputs)
    Block* contents = nested_contents(ifCall);

    bool firstBlock = true;
    List names;

    for (CaseIterator it(contents); it.unfinished(); it.advance()) {
        Block* caseBlock = nested_contents(it.current());

        if (firstBlock) {
            firstBlock = false;
            write_all_names_to_list(caseBlock, &names);
            continue;
        }

        // search through 'names' and remove any not in this block.
        for (int i=0; i < names.length(); i++) {
            if (is_null(names[i]))
                continue;
            if (caseBlock->get(as_cstring(names[i])) == NULL)
                set_null(names[i]);
        }
    }

    names.removeNulls();

    for (int i=0; i < names.length(); i++) {
        caValue* name = names[i];

        // Skip if name is already bound
        if (find_output_placeholder_with_name(contents, name) != NULL)
            continue;

        if_block_append_output(contents, name);
    }
}

void if_block_update_output_placeholder_types_from_cases(Term* ifBlock)
{
    Block* masterContents = nested_contents(ifBlock);

    for (int outputIndex=0;; outputIndex++) {
        Term* masterPlaceholder = get_output_placeholder(masterContents, outputIndex);
        if (masterPlaceholder == NULL)
            return;

        List types;

        // Iterate through each case, and collect the output types
        for (int i=0; i < masterContents->length(); i++) {
            Term* term = masterContents->get(i);
            if (term->function != FUNCS.case_func)
                continue;
            Term* placeholder = get_output_placeholder(nested_contents(term), outputIndex);
            ca_assert(placeholder != NULL);
            set_type(types.append(), placeholder->type);
        }

        change_declared_type(masterPlaceholder, find_common_type(&types));
    }
}

void finish_if_block(Term* ifBlock)
{
    Block* contents = nested_contents(ifBlock);

    // Make sure there is a primary output
    if (get_output_placeholder(contents, 0) == NULL)
        if_block_prepend_primary_output(ifBlock);

    // Turn name rebinds into outer name rebinds.
    for (CaseIterator it(contents); it.unfinished(); it.advance()) {
        Term* term = it.current();
        if_block_turn_outer_name_rebinds_into_outputs(ifBlock, nested_contents(term));
    }

    if_block_turn_common_rebinds_into_outputs(ifBlock);

    if_block_update_output_placeholder_types_from_cases(ifBlock);
    update_extra_outputs(ifBlock);
}

} // namespace circa
