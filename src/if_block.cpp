// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <set>

#include "common_headers.h"

#include "branch.h"
#include "code_iterators.h"
#include "kernel.h"
#include "building.h"
#include "evaluation.h"
#include "importing_macros.h"
#include "inspection.h"
#include "stateful_code.h"
#include "term.h"
#include "type.h"
#include "type_inference.h"
#include "update_cascades.h"

#include "if_block.h"

namespace circa {

static Term* if_block_add_input(Term* ifCall, Term* input);

struct CaseIterator
{
    BranchIteratorFlat branchIterator;

    CaseIterator(Branch* branch)
      : branchIterator(branch)
    {
        advanceWhileInvalid();
    }
     
    bool finished()
    {
        return branchIterator.finished();
    }
    void advance()
    {
        branchIterator.index++;
        advanceWhileInvalid();
    }
    void advanceWhileInvalid()
    {
    possibly_invalid:
        if (finished())
            return;

        if (branchIterator.current()->function != FUNCS.case_func) {
            branchIterator.advance();
            goto possibly_invalid;
        }
    }

    Term* current()
    {
        ca_assert(branchIterator.current()->function == FUNCS.case_func);
        return branchIterator.current();
    }
    int index()
    {
        return branchIterator.index;
    }

    bool unfinished() { return !finished(); }
    void operator++() { advance(); }
};

int if_block_count_cases(Branch* block)
{
    int result = 0;
    for (int i=0; i < block->length(); i++)
        if (block->get(i) != NULL && block->get(i)->function == FUNCS.case_func)
            result++;
    return result;
}

Term* if_block_add_input(Term* ifBlock, Term* input)
{
    Branch* contents = nested_contents(ifBlock);

    int existingInputCount = ifBlock->numInputs();

    Term* placeholder = append_input_placeholder(contents);
    rename(placeholder, input->nameSymbol);
    change_declared_type(placeholder, input->type);

    set_input(ifBlock, existingInputCount, input);

    // Add a corresponding input placeholder to each case
    for (CaseIterator it(contents); it.unfinished(); it.advance()) {
        Term* term = it.current();
        Branch* caseContents = nested_contents(term);
        Term* casePlaceholder = append_input_placeholder(caseContents);
        change_declared_type(casePlaceholder, placeholder->type);
        rename(casePlaceholder, input->nameSymbol);
    }

    return placeholder;
}

Term* if_block_prepend_primary_output(Term* ifBlock)
{
    Branch* contents = nested_contents(ifBlock);

    Term* placeholder = prepend_output_placeholder(contents, NULL);

    // Insert a corresponding output in each case.
    for (CaseIterator it(contents); it.unfinished(); it.advance()) {
        Branch* caseContents = nested_contents(it.current());

        Term* result = find_last_non_comment_expression(caseContents);

        // If the last term already has a name then don't make it the default output.
        if (result != NULL && !has_empty_name(result))
            result = NULL;

        prepend_output_placeholder(nested_contents(it.current()), result);
    }
    return placeholder;
}

Term* if_block_append_output(Branch* block, const char* nameString)
{
    Name name = name_from_string(nameString);

    Term* placeholder = append_output_placeholder(block, NULL);
    if (name != name_None)
        rename(placeholder, name);

    // Add a corresponding output placeholder to each case
    for (CaseIterator it(block); it.unfinished(); it.advance()) {

        Branch* caseContents = nested_contents(it.current());

        // Use the local name binding as the placeholder's result
        Term* result = find_name(caseContents, name);

        Term* casePlaceholder = append_output_placeholder(caseContents, result);
        if (name != name_None)
            rename(casePlaceholder, name);
    }

    return placeholder;
}

Term* if_block_get_case(Branch* block, int index)
{
    for (int i=0; i < block->length(); i++) {
        if (block->get(i) == NULL || block->get(i)->function != FUNCS.case_func)
            continue;

        if (index == 0)
            return block->get(i);

        index--;
    }
    return NULL;
}

void if_block_start(Branch* block)
{
    // Create a placeholder for primary output
    append_output_placeholder(block, NULL);
}

Term* if_block_append_case(Branch* block, Term* input)
{
    int insertPos = 0;
    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);

        if (term->function == FUNCS.input)
            insertPos = term->index + 1;

        // Insert position is right after the last non-default case.
        if (term->function == FUNCS.case_func && term->input(0) != NULL)
            insertPos = term->index + 1;
    }

    Term* newCase = apply(block, FUNCS.case_func, TermList(input));
    block->move(newCase, insertPos);

    // Add existing input placeholders to this case
    for (int i=0;; i++) {
        Term* placeholder = get_input_placeholder(block, i);
        if (placeholder == NULL) break;
        Term* localPlaceholder = append_input_placeholder(nested_contents(newCase));
        change_declared_type(localPlaceholder, placeholder->type);
        rename(localPlaceholder, placeholder->nameSymbol);
    }

    // Add existing output placeholders to this case
    for (int i=0;; i++) {
        Term* placeholder = get_output_placeholder(block, i);
        if (placeholder == NULL) break;
        Term* localPlaceholder = append_output_placeholder(nested_contents(newCase), NULL);
        change_declared_type(localPlaceholder, placeholder->type);
        rename(localPlaceholder, placeholder->nameSymbol);
    }

    return newCase;
}
bool is_case_branch(Branch* branch)
{
    return branch->owningTerm != NULL && branch->owningTerm->function == FUNCS.case_func;
}
bool is_if_block(Branch* branch)
{
    return branch->owningTerm != NULL && branch->owningTerm->function == FUNCS.if_block;
}
Branch* get_block_for_case_branch(Branch* branch)
{
    return get_parent_branch(branch);
}
Term* if_block_get_output_by_name(Branch* block, const char* name)
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

void if_block_finish_appended_case(Branch* block, Term* caseTerm)
{
    // Add an output placeholder
    apply(nested_contents(caseTerm), FUNCS.output,
        TermList(find_last_non_comment_expression(nested_contents(caseTerm))));
}

void append_state_placeholders_if_needed(Branch* branch)
{
    if (!has_state_input(branch))
        append_state_input(branch);
    if (!has_state_output(branch))
        append_state_output(branch);
}

void if_block_normalize_state_inputs(Term* ifBlock)
{
    Branch* contents = nested_contents(ifBlock);

    // Check if any branches have a state input
    bool anyState = false;
    for (CaseIterator it(contents); it.unfinished(); it.advance()) {
        if (has_state_input(nested_contents(it.current())))
            anyState = true;
    }

    if (!anyState)
        return;

    append_state_placeholders_if_needed(contents);
    for (CaseIterator it(contents); it.unfinished(); it.advance())
        append_state_placeholders_if_needed(nested_contents(it.current()));
}

bool if_block_is_name_bound_in_every_case(Branch* contents, const char* name)
{
    for (CaseIterator it(contents); it.unfinished(); it.advance()) {
        if (!nested_contents(it.current())->contains(name))
            return false;
    }
    return true;
}

void if_block_create_input_placeholders_for_outer_pointers(Term* ifCall)
{
    Branch* contents = nested_contents(ifCall);
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
        rename(placeholder, outer->nameSymbol);

        // Go through each case and repoint to this new placeholder
        for (CaseIterator it(contents); it.unfinished(); it.advance()) {
            remap_pointers_quick(nested_contents(it.current()), outer, placeholder);
        }
    }
}

#if 0
void if_block_fix_outer_pointers(Term* ifCall, Branch* caseContents)
{
    Branch* contents = nested_contents(ifCall);

    for (OuterInputIterator it(caseContents); it.unfinished(); ++it) {

        // Don't worry about outer pointers to values. (This should probably be
        // standard behavior)
        if (is_value(it.currentTerm()))
            continue;

        // Fetch values from OuterInputIterator, while it's safe. The iterator
        // may get confused soon, due to inserted terms.
        Term* currentTerm = it.currentTerm();
        Term* input = it.currentInput();
        int currentInputIndex = it.currentInputIndex();

        // Check if this pointer goes outside the if-block. If so, we'll have to
        // find the corresponding placeholder (or create a new one).
        if (input->owningBranch != contents) {

            Term* placeholder = NULL;

            int inputIndex = find_input_index_for_pointer(ifCall, input);
            if (inputIndex >= 0) {
                placeholder = get_input_placeholder(contents, inputIndex);
            } else {
                // Create a new placeholder
                // This call will result in an inserted term, which will confuse
                // our OuterInputIterator. So, don't use the iterator again until
                // the next iteration.
                placeholder = if_block_add_input(ifCall, input);
            }

            input = placeholder;
        }

        // Now 'input' points to an if-block placeholder, remap it to the case-local
        // placeholder.
        Term* caseLocal = get_input_placeholder(caseContents, input->index);
        ca_assert(caseLocal != NULL);
        set_input(currentTerm, currentInputIndex, caseLocal);
    }
}
#endif

void if_block_turn_outer_name_rebinds_into_outputs(Term* ifCall, Branch *caseBranch)
{
    Branch* mainBranch = nested_contents(ifCall);
    Branch* outerBranch = ifCall->owningBranch;

    for (int i=0; i < caseBranch->length(); i++) {
        Term* term = caseBranch->get(i);
        if (term->name == "")
            continue;

        Name name = term->nameSymbol;

        Term* outer = find_name(outerBranch, name);
        if (outer == NULL)
            continue;

        // This term rebinds an outer name.

        // First, bring in the outer name as an input to the branch.

        // Check if we already have an output for this name.
        Term* inputPlaceholder = find_input_placeholder_with_name(mainBranch, name_to_string(name));

        // Create it if necessary
        if (inputPlaceholder == NULL) {
            inputPlaceholder = if_block_add_input(ifCall, outer);
            rename(inputPlaceholder, name);

            // Fix the new input placeholders to have the correct name and input.
            for (CaseIterator it(mainBranch); it.unfinished(); it.advance()) {
                Branch* caseContents = nested_contents(it.current());
                Term* casePlaceholder = get_input_placeholder(caseContents,
                    inputPlaceholder->index);
                ca_assert(casePlaceholder != NULL);
                rename(casePlaceholder, name);
            }
        }

        // Now make sure there is an output placeholder for this name.
        Term* outputPlaceholder = find_output_placeholder_with_name(mainBranch, name_to_string(name));

        if (outputPlaceholder == NULL)
            outputPlaceholder = if_block_append_output(mainBranch, name_to_string(name));
    }
}

void write_all_names_to_list(Branch* branch, List* names)
{
    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);
        if (term->name != "")
            set_string(names->append(), term->name);
    }
}

void if_block_turn_common_rebinds_into_outputs(Term* ifCall)
{
    // Find names which are bound in every branch (and not already outputs)
    Branch* contents = nested_contents(ifCall);

    bool firstBranch = true;
    List names;

    for (CaseIterator it(contents); it.unfinished(); it.advance()) {
        Branch* caseBranch = nested_contents(it.current());

        if (firstBranch) {
            firstBranch = false;
            write_all_names_to_list(caseBranch, &names);
            continue;
        }

        // search through 'names' and remove any not in this branch.
        for (int i=0; i < names.length(); i++) {
            if (is_null(names[i]))
                continue;
            if (caseBranch->get(as_cstring(names[i])) == NULL)
                set_null(names[i]);
        }
    }

    names.removeNulls();

    for (int i=0; i < names.length(); i++) {
        const char* name = as_cstring(names[i]);

        // Skip if name is already bound
        if (find_output_placeholder_with_name(contents, name) != NULL)
            continue;

        if_block_append_output(contents, name);
    }
}

void if_block_update_output_placeholder_types_from_cases(Term* ifBlock)
{
    Branch* masterContents = nested_contents(ifBlock);

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

void modify_branch_so_that_state_access_is_indexed(Branch* branch, int index)
{
    Term* stateInput = find_state_input(branch);
    if (stateInput == NULL)
        return;

    // Create terms for unpack_state_from_list and pack_state_to_list, if needed.

    Term* stateOutput = find_state_output(branch);

    // If state output is directly connected to input, then we don't need to create
    // an unpack_state_from_list call.
    if (stateOutput->input(0) == stateInput) {
        // State output is directly connected to input. In this case, the branch should
        // null out the state field.
        Term* stateResult = create_value(branch, &NULL_T);
        Term* packList = apply(branch, FUNCS.pack_state_to_list,
            TermList(stateInput, stateResult));
        packList->setIntProp("index", index);
        set_input(stateOutput, 0, packList);
        return;
    }

    // There are terms between state input & output, bracket them with calls to
    // unpack_state_from_list and pack_state_to_list.
    Term* unpackList = apply(branch, FUNCS.unpack_state_from_list, TermList(stateInput));
    unpackList->setIntProp("index", index);
    move_after_inputs(unpackList);

    // Grab a copy of stateInput's users (before it's modified)
    TermList stateInputUsers = stateInput->users;
    for (int i=0; i < stateInputUsers.length(); i++) {
        Term* term = stateInputUsers[i];
        if (term == unpackList)
            continue;
        remap_pointers_quick(term, stateInput, unpackList);
    }

    Term* stateResult = stateOutput->input(0);
    ca_assert(stateResult != NULL);

    Term* packList = apply(branch, FUNCS.pack_state_to_list,
        TermList(stateInput, stateResult));
    packList->setIntProp("index", index);
    packList->setBoolProp("final", true);
    set_input(stateOutput, 0, packList);
    move_after(packList, stateResult);
}

void finish_if_block(Term* ifBlock)
{
    Branch* contents = nested_contents(ifBlock);

    // Make sure there is a primary output
    if (get_output_placeholder(contents, 0) == NULL)
        if_block_prepend_primary_output(ifBlock);

    if_block_normalize_state_inputs(ifBlock);

    // Turn name rebinds into outer name rebinds.
    for (CaseIterator it(contents); it.unfinished(); it.advance()) {
        Term* term = it.current();
        if_block_turn_outer_name_rebinds_into_outputs(ifBlock, nested_contents(term));
    }

    // Fix state in each case.
    int caseIndex = 0;
    for (CaseIterator it(contents); it.unfinished(); it.advance()) {
        Term* term = it.current();
        modify_branch_so_that_state_access_is_indexed(nested_contents(term), caseIndex);
        caseIndex++;
    }

    if_block_turn_common_rebinds_into_outputs(ifBlock);

    if_block_update_output_placeholder_types_from_cases(ifBlock);
    check_to_insert_implicit_inputs(ifBlock);
    update_extra_outputs(ifBlock);
}

} // namespace circa
