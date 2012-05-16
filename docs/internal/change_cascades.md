
This is an attempt to document all the types of code changes which can cause
cascading changes to other parts of the data. This document is not complete.

By "adding a call to X" we mean changing a term's function to X, including
a creation of a new term with function X.

"Removing a call to X" similarly means changing a term's function from X to
something else.

Adding or removing a name binding
    If this occurs in a branch with an exit point
        Can: cause the exit_point inputs to change
    If this occurs in a minor branch, and the name is bound in a parent branch
        Can: cause an implicit name rebind in the parent branch
    If the name is bound to an open state variable
        Can: create a new pack_state() term, or cause an existing pack_state() to move

Add/remove a call to an 'exiting' function (continue/break/return/discard)
    Can: create an implicit exit_point() term

Add an exit_point() term
    If it occurs in a minor branch, can add an exit_point() in parent term.
    If the branch has implicit state, can create a pack_state() term

Add a declared_state() term
    Can: create unpack_state and pack_state calls

Adding a pack_state() term
    Create state input and output placeholders in the current branch
    Add an implicit input and output to the owning term
    Add a pack_state() term for the owning term
