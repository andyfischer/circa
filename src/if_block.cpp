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

    int firstCaseLocation = 0;
    int firstJoinLocation = contents->length();

    // For each name, create a term that selects the correct version of this name.
    for (std::set<std::string>::const_iterator it = boundNames.begin();
            it != boundNames.end();
            ++it)
    {
        std::string const& name = *it;

        TermList inputs;

        Term* outerVersion = get_named_at(ifCall, name);

        for (int i = 0; ; i++) {
            Term* caseTerm = contents->getSafe(firstCaseLocation + i);
            if (caseTerm == NULL
                || (caseTerm->function != CASE_FUNC) && (caseTerm->function != BRANCH_FUNC))
                break;

            Term* local = caseTerm->contents(name.c_str());
            inputs.append(local == NULL ? outerVersion : local);
        }

        apply(contents, JOIN_FUNC, inputs, name);
    }

    // Append output_placeholder() terms
    int lastJoinLocation = contents->length();

    for (int i=firstJoinLocation; i < lastJoinLocation; i++) {
        Term* placeholder = apply(contents, OUTPUT_PLACEHOLDER_FUNC,
            TermList(contents->get(i)), contents->get(i)->name);
        change_declared_type(placeholder, contents->get(i)->type);
    }

    // Append the primary output
    apply(contents, OUTPUT_PLACEHOLDER_FUNC, TermList(NULL));
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
            for (int j=0; j < acceptedBranch->length(); j++) {
                evaluate_single_term(context, acceptedBranch->get(j));
                if (evaluation_interrupted(context))
                    break;
            }

            // Save result
            Term* outputTerm = find_last_non_comment_expression(acceptedBranch);
            if (outputTerm != NULL)
                copy(outputTerm, &output);

            acceptedBranchIndex = branchIndex;
            break;
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

    pop_frame(context);

    swap(&output, OUTPUT);
}

} // namespace circa
