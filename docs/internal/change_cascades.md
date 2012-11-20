
This is an attempt to document all the types of code changes which can cause
cascading changes to other parts of the data. This document is not complete.

Event: Changing the name of T

  Effect: Change existing named references to point to T
    Requirement: T now has a non-blank name
    Find the term E which was the existing name binding at location T
    Look at all users of E
    For any that have T visible, repoint to T

  Effect: Possibly remove existing references to T
    Requirement: T previously had a non-blank name
    Look at all users of T
    For any name-based reference, point that ref to the existing name binding at location T
      
  Effect: update exit_point
    For all exit_points that are after T, in the same block
      Update all inputs

  Effect: update pack_state
    For all pack_states that are after T, in the same block
      Update all inputs

  Effect: add outer name rebind
    Requirement: T is inside a minor block
    Requirement: T's new name is bound in the parent block
    Requirement: T's new name is not already an output of this minor block
    Add this name as an output of this minor block

  Effect: add or move pack_state call
    Requirement: there is a declared state variable in this block with this name
    Either create a new pack_state, or move the existing pack_state after this term.

Event: Create an exit_point() term
    If it occurs in a minor block, can add an exit_point() in parent term.
    If the block has implicit state, can create a pack_state() term

Event: Adding a pack_state() term
  Effect: Make the current block stateful
    Requirement: Current block is not already stateful
    Create state input and output placeholders in the current block
    Add an implicit input and output to the owning term
    Create a pack_state() term for the owning term

Event: Create a declared_state() term
    Can create unpack_state and pack_state calls

