// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "circa.h"

namespace circa {

void update_if_block_joining_branch(Term* ifCall)
{
    Branch& contents = ifCall->asBranch();

    // Create the joining contents if necessary
    if (!contents.contains("#joining"))
        create_branch(contents, "#joining");

    Branch& joining = contents["#joining"]->asBranch();
    joining.clear();

    // This is used later.
    Term* satisfiedIndex = int_value(joining, 0, "#satisfiedIndex");

    // Find the set of all names bound in every branch.
    std::set<std::string> boundNames;

    {
        for (int branch_index=0; branch_index < contents.length()-1; branch_index++) {
            Branch& branch = contents[branch_index]->asBranch();

            TermNamespace::const_iterator it;
            for (it = branch.names.begin(); it != branch.names.end(); ++it)
                boundNames.insert(it->first);
        }
    }

    Branch* outerScope = ifCall->owningBranch;
    assert(outerScope != NULL);

    // Filter out some names from boundNames.
    for (std::set<std::string>::iterator it = boundNames.begin(); it != boundNames.end();)
    {
        std::string const& name = *it;

        // Ignore hidden names
        if ((name[0] == '#') && (name != "#out")) {
            boundNames.erase(it++);
            continue;
        }

        // We only rebind names that are either 1) already bound in the outer scope, or
        // 2) bound in every possible branch.
        
        bool boundInOuterScope = find_named(*outerScope, name) != NULL;

        int numberOfBranchesWithThisName = 0;
        bool boundInEveryBranch = true;

        for (int branch_index=0; branch_index < contents.length()-1; branch_index++) {
            if (contents[branch_index]->asBranch().contains(name))
                numberOfBranchesWithThisName++;
            else
                boundInEveryBranch = false;
        }

        if (!boundInOuterScope && !boundInEveryBranch)
            boundNames.erase(it++);
        else
            ++it;
    }

    // For each name, create a term that selects the correct version of this name.
    for (std::set<std::string>::const_iterator it = boundNames.begin();
            it != boundNames.end();
            ++it)
    {
        std::string const& name = *it;

        // Make a list where we find the corresponding term for this name in each branch.
        RefList selections;

        for (int branch_index=0; branch_index < contents.length()-1; branch_index++) {
            Branch& branch = contents[branch_index]->asBranch();

            if (branch.contains(name))
                selections.append(branch[name]);
            else
                selections.append(find_named(*outerScope, name));
        }

        Term* selection_list = apply(joining, LIST_FUNC, selections);

        // Populate this list immediately so that type inference can work
        evaluate_term(selection_list);

        Term* joiningTerm = apply(joining, GET_INDEX_FUNC,
                RefList(selection_list, satisfiedIndex), name);

        // Bind this names in the outer branch
        // TODO: This only works when the if call is the last thing in this branch (ie,
        // at parse time). It will cause problems if we call update_if_block_joining_branch
        // at a later time and there have been other name bindings since then. Need a
        // better solution for this.
        outerScope->bindName(joiningTerm, name);
    }
}

} // namespace circa
