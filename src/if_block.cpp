// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <set>

#include "common_headers.h"

#include "branch.h"
#include "code_iterators.h"
#include "kernel.h"
#include "building.h"
#include "evaluation.h"
#include "importing_macros.h"
#include "locals.h"
#include "refactoring.h"
#include "stateful_code.h"
#include "term.h"
#include "type.h"
#include "type_inference.h"
#include "update_cascades.h"

#include "if_block.h"

namespace circa {

void if_block_update_case_placeholders_from_master(Term* ifCall, Term* caseTerm);
void if_block_repoint_outer_pointers_to_existing_placeholders(Term* ifCall);

int if_block_count_cases(Term* term)
{
    Branch* contents = nested_contents(term);
    int result = 0;
    for (int i=0; i < contents->length(); i++)
        if (contents->get(i) != NULL && contents->get(i)->function == CASE_FUNC)
            result++;
    return result;
}

Term* if_block_get_case(Term* term, int index)
{
    Branch* contents = nested_contents(term);
    for (int i=0; i < contents->length(); i++) {
        if (contents->get(i) == NULL || contents->get(i)->function != CASE_FUNC)
            continue;

        if (index == 0)
            return contents->get(i);

        index--;
    }
    return NULL;
}

Term* if_block_append_case(Term* ifBlock, Term* input)
{
    Branch* contents = nested_contents(ifBlock);
    int insertPos = 0;
    for (int i=0; i < contents->length(); i++) {
        Term* term = contents->get(i);

        // Insert position is right after the last non-default case.
        if (term->function == CASE_FUNC && term->input(0) != NULL)
            insertPos = term->index + 1;
    }

    Term* newCase = apply(contents, CASE_FUNC, TermList(input));
    contents->move(newCase, insertPos);
    return newCase;
}

void if_block_finish_appended_case(Term* ifBlock, Term* caseTerm)
{
    if_block_update_case_placeholders_from_master(ifBlock, caseTerm);
    if_block_repoint_outer_pointers_to_existing_placeholders(ifBlock);
}

bool if_block_is_name_bound_in_every_case(Branch* contents, const char* name)
{
    for (int i=0; i < contents->length(); i++) {
        Term* term = contents->get(i);
        if (term->function == CASE_FUNC)
            if (!nested_contents(term)->contains(name))
                return false;
    }
    return true;
}

void if_block_create_input_placeholders_for_outer_pointers(Term* ifCall)
{
    Branch* contents = nested_contents(ifCall);
    TermList outerTerms;

    // Find outer pointers across each case
    for (int i=0; i < contents->length(); i++) {
        Term* term = contents->get(i);
        if (term->function != CASE_FUNC)
            continue;

        list_outer_pointers(nested_contents(term), &outerTerms);
    }

    ca_assert(ifCall->numInputs() == 0);

    // Create input placeholders and add inputs for all outer pointers
    for (int i=0; i < outerTerms.length(); i++) {
        Term* outer = outerTerms[i];

        set_input(ifCall, i, outer);
        Term* placeholder = append_input_placeholder(nested_contents(ifCall));

        // Go through each case and repoint to this new placeholder
        for (int j=0; j < contents->length(); j++) {
            Term* term = contents->get(j);
            if (term->function != CASE_FUNC)
                continue;

            remap_pointers_quick(nested_contents(term), outer, placeholder);
        }
    }
}

void if_block_repoint_outer_pointers_to_existing_placeholders(Term* ifCall)
{
    Branch* contents = nested_contents(ifCall);

    for (int i=0; i < contents->length(); i++) {
        Term* term = contents->get(i);
        if (term->function != CASE_FUNC)
            continue;

        Branch* caseContents = nested_contents(term);

        for (OuterInputIterator it(caseContents); it.unfinished(); it.advance()) {
            Term* input = it.currentInput();

            // If this input is outside the if-block then find the corresponding
            // placeholder inside the block.
            if (input->owningBranch != contents) {
                int inputIndex = find_input_index_for_pointer(ifCall, input);
                ca_assert(inputIndex != -1);

                // Redirect to if-block placeholder (this will get redirected again
                // to the case-local placeholder).
                input = get_input_placeholder(contents, inputIndex);
                ca_assert(input != NULL);
            }

            // This input should now be one of the if-block placeholders
            ca_assert(input == get_input_placeholder(contents, input->index));

            Term* caseLocal = get_input_placeholder(caseContents, input->index);
            ca_assert(caseLocal != NULL);

            set_input(it.currentTerm(), it.currentInputIndex(), caseLocal);
        }
    }
}

void if_block_update_master_placeholders(Term* ifCall)
{
    Branch* contents = nested_contents(ifCall);

    // Find the set of all names that are bound in every branch.
    std::set<std::string> boundNames;
    bool hasState = false;

    for (int i=0; i < contents->length(); i++) {
        Term* term = contents->get(i);

        if (term->function == INPUT_PLACEHOLDER_FUNC)
            continue;
        if (term->function != CASE_FUNC)
            break;

        Branch* branch = nested_contents(term);

        hasState = hasState || find_state_input(branch) != NULL;

        TermNamespace::const_iterator it;
        for (it = branch->names.begin(); it != branch->names.end(); ++it) {
            std::string const& name = it->first;

            // Ignore empty or hidden names
            if (name == "" || name[0] == '#')
                continue;

            boundNames.insert(it->first);
        }
    }

    Branch* outerScope = ifCall->owningBranch;
    ca_assert(outerScope != NULL);

    // Filter out some names from boundNames.
    for (std::set<std::string>::iterator it = boundNames.begin(); it != boundNames.end();)
    {
        std::string const& name = *it;

        // We only rebind names that are either 1) already bound in the outer scope, or
        // 2) bound in every possible branch.
        
        bool boundInOuterScope = find_name(outerScope, name.c_str()) != NULL;

        bool boundInEveryBranch = if_block_is_name_bound_in_every_case(contents, name.c_str());;

        if (!boundInOuterScope && !boundInEveryBranch)
            boundNames.erase(it++);
        else
            ++it;
    }

    // Add each name as an input and output to the if_block
    int inputIndex = 0;
    for (std::set<std::string>::const_iterator it = boundNames.begin();
            it != boundNames.end();
            ++it) {

        std::string const& name = *it;
        Term* outer = get_named_at(ifCall, name);
        set_input(ifCall, inputIndex, outer);

        Term* placeholder = apply(contents, INPUT_PLACEHOLDER_FUNC, TermList(), name);
        if (outer != NULL)
            change_declared_type(placeholder, outer->type);
        contents->move(placeholder, inputIndex);
        inputIndex++;

        apply(contents, OUTPUT_PLACEHOLDER_FUNC, TermList(NULL), name);
    }

    // If any branches have a state input, then add a master state input too.
    if (hasState) {
        insert_state_input(contents);
        insert_state_output(contents);
    }

    // Finally, add a primary output
    apply(contents, OUTPUT_PLACEHOLDER_FUNC, TermList(NULL));
}

bool does_output_placeholder_exist(Branch* branch, Term* placeholder)
{
    for (int i=0;; i++) {
        Term* existingPlaceholder = get_output_placeholder(branch, i);
        if (existingPlaceholder == NULL)
            return false;

        if (is_state_input(placeholder) && is_state_input(existingPlaceholder))
            return true;
    }
}

void if_block_update_case_placeholders_from_master(Term* ifCall, Term* caseTerm)
{
    Branch* masterContents = nested_contents(ifCall);
    Branch* caseContents = nested_contents(caseTerm);

    // Create input placeholders
    for (int i=0;; i++) {
        Term* masterPlaceholder = get_input_placeholder(masterContents, i);
        if (masterPlaceholder == NULL)
            break;

        if (is_state_input(masterPlaceholder) && has_state_input(caseContents))
            continue;

        Term* placeholder = apply(caseContents, INPUT_PLACEHOLDER_FUNC, TermList(),
            masterPlaceholder->name);
        change_declared_type(placeholder, masterPlaceholder->type);
        copy(&masterPlaceholder->properties, &placeholder->properties);
        caseContents->move(placeholder, i);
        remap_pointers_quick(caseContents, masterPlaceholder, placeholder);
    }

    // Create output placeholders
    for (int i=0;; i++) {
        Term* masterPlaceholder = get_output_placeholder(masterContents, i);
        if (masterPlaceholder == NULL)
            break;

        if (does_output_placeholder_exist(caseContents, masterPlaceholder))
            continue;

        Term* nameResult = NULL;
        
        if (masterPlaceholder->name != "")
            nameResult = get_named_at(caseContents, caseContents->length(),
                masterPlaceholder->name.c_str());

        // Find appropriate result for state output
        if (is_state_input(masterPlaceholder))
            nameResult = find_open_state_result(caseContents, caseContents->length());

        // Find the appropriate result for the primary output
        if (i == 0 && masterPlaceholder->name == "")
            nameResult = find_last_non_comment_expression(caseContents);

        Term* placeholder = apply(caseContents, OUTPUT_PLACEHOLDER_FUNC,
            TermList(nameResult), masterPlaceholder->name);
        caseContents->move(placeholder, caseContents->length() - i - 1);
        if (nameResult != NULL)
            change_declared_type(placeholder, nameResult->type);
        copy(&masterPlaceholder->properties, &placeholder->properties);
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
            if (term->function != CASE_FUNC)
                continue;
            Term* placeholder = get_output_placeholder(nested_contents(term), outputIndex);
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

    Term* unpackList = apply(branch, BUILTIN_FUNCS.unpack_state_list, TermList(stateInput));
    unpackList->setIntProp("index", index);
    move_after_inputs(unpackList);

    for (int i=0; i < stateInput->users.length(); i++) {
        Term* term = stateInput->users[i];
        if (term == unpackList)
            continue;
        remap_pointers_quick(term, stateInput, unpackList);
    }

    Term* stateOutput = find_state_output(branch);
    ca_assert(stateOutput->input(0) != stateInput);

    Term* packList = apply(branch, BUILTIN_FUNCS.pack_state_to_list,
        TermList(stateInput, stateOutput->input(0)));
    packList->setIntProp("index", index);
    set_input(stateOutput, 0, packList);
}

void finish_if_block(Term* ifBlock)
{
    if_block_create_input_placeholders_for_outer_pointers(ifBlock);
    if_block_update_master_placeholders(ifBlock);

    Branch* contents = nested_contents(ifBlock);
    int caseIndex = 0;
    for (int i=0; i < contents->length(); i++) {
        Term* term = contents->get(i);
        if (term->function == CASE_FUNC) {
            if_block_update_case_placeholders_from_master(ifBlock, term);
            modify_branch_so_that_state_access_is_indexed(nested_contents(term), caseIndex);
            caseIndex++;
        }
    }

    if_block_update_output_placeholder_types_from_cases(ifBlock);
    check_to_insert_implicit_inputs(ifBlock);
}

CA_FUNCTION(evaluate_if_block)
{
    Term* caller = CALLER;
    EvalContext* context = CONTEXT;
    Branch* contents = nested_contents(caller);

    Branch* acceptedBranch = NULL;

    TaggedValue output;

    TaggedValue localState;
    TaggedValue prevScopeState;

    int termIndex = 0;
    while (contents->get(termIndex)->function == INPUT_PLACEHOLDER_FUNC)
        termIndex++;

    for (; termIndex < contents->length(); termIndex++) {
        Term* caseTerm = contents->get(termIndex);

        //std::cout << "checking: " << get_term_to_string_extended(caseTerm) << std::endl;
        //std::cout << "with stack: " << STACK->toString() << std::endl;

        // Look at input
        TaggedValue inputIsn;
        write_input_instruction(caseTerm, caseTerm->input(0), &inputIsn);
        
        if (caseTerm->input(0) == NULL || as_bool(get_arg(context, &inputIsn))) {

            acceptedBranch = caseTerm->nestedContents;

            ca_assert(acceptedBranch != NULL);

            // Copy inputs
            List registers;
            registers.resize(acceptedBranch->length());
            int numInputs = NUM_INPUTS;
            for (int inputIndex=0; inputIndex < numInputs; inputIndex++)
                copy(INPUT(inputIndex), registers[inputIndex]);

            push_frame(context, acceptedBranch, &registers);

            // Evaluate each term
            for (int i=0; i < acceptedBranch->length(); i++) {
                evaluate_single_term(context, acceptedBranch->get(i));
                if (evaluation_interrupted(context))
                    break;
            }

            if (context->errorOccurred)
                return;

            swap(&registers, &top_frame(context)->registers);
            pop_frame(context);

            // Save outputs
            for (int i=0;; i++) {
                Term* placeholder = get_output_placeholder(acceptedBranch, i);
                if (placeholder == NULL)
                    break;

                copy(registers[placeholder->index], OUTPUT_NTH(i));
            }

            return;
        }
    }
}

} // namespace circa
