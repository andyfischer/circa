// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "circa.h"

namespace circa {

void update_if_block_joining_branch(Term* ifCall)
{
    Branch& branch = ifCall->asBranch();

    // Create the joining branch if necessary
    if (!branch.contains("#joining"))
        create_branch(&branch, "#joining");

    Term* condition = ifCall->input(0);
    Branch& positiveBranch = branch["if"]->asBranch();
    Branch& joiningBranch = branch["#joining"]->asBranch();

    joiningBranch.clear();

    // 'else' branch is optional
    Branch* elseBranch = NULL;
    if (branch.contains("else"))
        elseBranch = &branch["else"]->asBranch();

    // Get a list of all names bound in this branch
    std::set<std::string> boundNames;

    {
        TermNamespace::const_iterator it;
        for (it = positiveBranch.names.begin(); it != positiveBranch.names.end(); ++it)
            boundNames.insert(it->first);

        if (elseBranch != NULL) {
            for (it = elseBranch->names.begin(); it != elseBranch->names.end(); ++it)
                boundNames.insert(it->first);
        }
    }

    // Ignore any names which are not bound in the outer branch
    Branch* outerScope = ifCall->owningBranch;
    assert(outerScope != NULL);

    for (std::set<std::string>::iterator it = boundNames.begin(); it != boundNames.end();)
    {
        std::string const& name = *it;

        if (find_named(outerScope, name) == NULL)
            boundNames.erase(it++);

        // Also ignore hidden names
        else if ((name[0] == '#') && (name != OUTPUT_PLACEHOLDER_NAME))
            boundNames.erase(it++);
        else
            ++it;
    }

    // For each name, create a joining term
    {
        std::set<std::string>::const_iterator it;
        for (it = boundNames.begin(); it != boundNames.end(); ++it)
        {
            std::string const& name = *it;

            Term* outerVersion = find_named(outerScope, name);
            Term* positiveVersion = outerVersion;
            Term* negativeVersion = outerVersion;

            if (positiveBranch.contains(name))
                positiveVersion = positiveBranch[name];

            if (elseBranch != NULL && elseBranch->contains(name))
                negativeVersion = elseBranch->get(name);

            Term* joining = apply(&joiningBranch, "if_expr",
                    RefList(condition, positiveVersion, negativeVersion));

            // Bind these names in the outer branch
            // TODO: This only works when the if call is the last thing in this branch (ie,
            // at parse time). It will cause problems if we call update_if_block_joining_branch
            // at a later time and there have been other name bindings since then. Need a
            // better solution for this.
            branch.bindName(joining, name);
        }
    }
}

}
