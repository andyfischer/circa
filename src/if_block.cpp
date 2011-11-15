// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <set>

#include "common_headers.h"

#include "branch.h"
#include "kernel.h"
#include "building.h"
#include "evaluation.h"
#include "importing_macros.h"
#include "locals.h"
#include "stateful_code.h"
#include "term.h"
#include "type.h"
#include "type_inference.h"
#include "update_cascades.h"

#include "if_block.h"

namespace circa {

void finish_if_block(Term* ifCall)
{
    Branch* contents = nested_contents(ifCall);

    // Find the set of all names bound in every branch.
    std::set<std::string> boundNames;

    for (int branch_index=0; branch_index < contents->length()-1; branch_index++) {
        Term* term = contents->get(branch_index);
        Branch* branch = nested_contents(term);

        TermNamespace::const_iterator it;
        for (it = branch->names.begin(); it != branch->names.end(); ++it) {
            std::string const& name = it->first;

            // Ignore empty or hidden names
            if (name == "" || name[0] == '#') {
                continue;
            }

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

        bool boundInEveryBranch = true;

        for (int branch_index=0; branch_index < contents->length()-1; branch_index++) {
            Branch* branch = nested_contents(contents->get(branch_index));
            if (!branch->contains(name))
                boundInEveryBranch = false;
        }

        if (!boundInOuterScope && !boundInEveryBranch)
            boundNames.erase(it++);
        else
            ++it;
    }

    int numOutputs = boundNames.size() + 1;

    // For each case branch, create a list of output_placeholder() terms. This list
    // should be equivalent across all cases.
    for (int caseIndex=0;; caseIndex++) {
        Term* caseTerm = contents->getSafe(caseIndex);
        if (caseTerm == NULL
                || (caseTerm->function != CASE_FUNC && caseTerm->function != BRANCH_FUNC))
            break;

        for (std::set<std::string>::const_iterator it = boundNames.begin();
                it != boundNames.end();
                ++it) {
            std::string const& name = *it;

            Term* input = find_name(nested_contents(caseTerm), name.c_str());
            Term* placeholder = apply(nested_contents(caseTerm), OUTPUT_PLACEHOLDER_FUNC,
                TermList(input), name);
            change_declared_type(placeholder, input->type);
        }

        // Also add an output_placeholder for the primary output.
        apply(nested_contents(caseTerm), OUTPUT_PLACEHOLDER_FUNC, TermList(NULL));
    }

    // Now that each case branch has an output_placeholder list, create a master list
    // in the above if_block branch.
    for (int outputIndex=0; outputIndex < numOutputs; outputIndex++) {

        List typeList;
        std::string name;

        for (int caseIndex=0;; caseIndex++) {
            Term* caseTerm = contents->getSafe(caseIndex);
            if (caseTerm == NULL || (caseTerm->function != CASE_FUNC
                    && caseTerm->function != BRANCH_FUNC))
                break;

            Term* placeholder = caseTerm->contents()->getFromEnd(numOutputs - outputIndex - 1);
            set_type(typeList.append(), placeholder->type);
            name = placeholder->name;
        }

        Term* masterPlaceholder = apply(contents, OUTPUT_PLACEHOLDER_FUNC, TermList(NULL), name);
        change_declared_type(masterPlaceholder, find_common_type(&typeList));
    }
}

int if_block_num_branches(Term* ifCall)
{
    return nested_contents(ifCall)->length() - 1;
}
Branch* if_block_get_branch(Term* ifCall, int index)
{
    return ifCall->contents(index)->contents();
}

CA_FUNCTION(evaluate_if_block)
{
    Term* caller = CALLER;
    EvalContext* context = CONTEXT;
    Branch* contents = nested_contents(caller);

    int numBranches = contents->length() - 1;
    int acceptedBranchIndex = 0;
    Branch* acceptedBranch = NULL;

    TaggedValue output;

    TaggedValue localState;
    TaggedValue prevScopeState;

    for (int branchIndex=0; branchIndex < numBranches; branchIndex++) {
        Term* branch = contents->get(branchIndex);

        //std::cout << "checking: " << get_term_to_string_extended(branch) << std::endl;
        //std::cout << "with stack: " << STACK->toString() << std::endl;

        // Look at input
        TaggedValue inputIsn;
        write_input_instruction(branch, branch->input(0), &inputIsn);
        
        if (branch->input(0) == NULL || as_bool(get_arg(context, &inputIsn))) {

            acceptedBranch = branch->nestedContents;

            ca_assert(acceptedBranch != NULL);

            push_frame(context, acceptedBranch);

            // Evaluate each term
            for (int i=0; i < acceptedBranch->length(); i++) {
                evaluate_single_term(context, acceptedBranch->get(i));
                if (evaluation_interrupted(context))
                    break;
            }

            List registers;
            swap(&registers, &top_frame(context)->registers);
            pop_frame(context);

            // Save primary output
            Term* outputTerm = find_last_non_comment_expression(acceptedBranch);
            if (outputTerm != NULL)
                copy(registers[outputTerm->index], OUTPUT);

            // Save extra outputs
            for (int i=1;; i++) {
                Term* placeholder = get_output_placeholder(acceptedBranch, i);
                if (placeholder == NULL)
                    break;

                copy(registers[placeholder->input(0)->index], EXTRA_OUTPUT(i-1));
            }

            return;
        }
    }

    // Evaluate all join() terms
#if 0
    for (int i=0; i < contents->length(); i++) {
        Term* term = contents->get(i);
        if (term->function == JOIN_FUNC) {
            TaggedValue inputIsn;
            write_input_instruction(term, term->input(acceptedBranchIndex), &inputIsn);
            TaggedValue* value = get_arg(context, &inputIsn);

            // Write the result value to the corresponding place in an above frame.
            Term* outputTerm = caller->owningBranch->get(caller->index + 1 + i);
            Frame* upperFrame = get_frame(CONTEXT, 1);

            copy(value, upperFrame->registers[outputTerm->index]);
        }
    }
#endif

    // Copy joined values to output slots
#if 0
    Branch* joining = nested_contents(contents->get(contents->length()-1));

    for (int i=0; i < joining->length(); i++) {
        Term* joinTerm = joining->get(i);

        // Fetch the result value using an input instruction
        TaggedValue inputIsn;
        write_input_instruction(joinTerm, joinTerm->input(acceptedBranchIndex), &inputIsn);
        TaggedValue* value = get_arg(context, &inputIsn);

        // Write the result value to the corresponding place in an above frame.
        Term* outputTerm = caller->owningBranch->get(caller->index + 1 + i);
        Frame* upperFrame = get_frame(CONTEXT, 1);

        copy(value, upperFrame->registers[outputTerm->index]);
    }
#endif
}

} // namespace circa
