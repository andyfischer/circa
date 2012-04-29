
# The Code Graph #

In Circa, most compiled code is part of one global nested tree. For lack of a better
name we'll refer to it as the "code graph", since it's similar to a scene graph.

The data structures involved are Branches and Terms; a Term can be thought of as a Node,
and a Branch can be thought of as a list of Terms.

The graph is organized as:

  [Root branch]
    [Module 1]
      [Functions]
      [Types]
      [Global terms]
    [Module 2]
    [...]


There's a root Branch (also called the Kernel). Inside the branch are Modules. Usually
each Module corresponds to one script file, but that is not a rule. Inside each Module
is the module's code.

For any Term found within the graph, we can construct a global name which uniquely
identifies this term. Within each branch, every term has a locally-unique name. So, the
global name is constructed by concatenating these local names.

(1) It's possible to have Branches and Terms that are not part of a code graph. For these,
there is no global name.

For a given World, there is one global code graph that represents the most recent version
of all code. The structures in this graph will change as code is modified.

# Live links #

When operating on code, we often want to have values that reference specific Branches
or Terms. And, we want these to always reference the most recent version of these
structures. So for this we have "live links", which don't store a link to the actual
Branch or Term data structure, instead they store the global name. As the code is
modified, the link will always refer to the latest code with that name.

Due to (1) above, not all terms have global names, so not all terms can have a live link.

Since terms may be deleted, a live link may become null after a code modification.
A live link may also point to the wrong thing after an unstructured change.

Code modification operations should endevour to preserve unique names. If a branch is
created to replace an existing branch, the new branch should use unique names that best
relate to the existing one. This way, live links have the best odds of remaining accurate.

# Versioning #

Something to consider for the future is versioning. Maybe every global name can have a
version number attached. And, old versions of the code graph are stored, so that we can
always find an older piece of code using the correct global name and version number.

In this scheme, we have more information available when translating an older link to
a newer link. In the easy case, we just modify the global name's version number and
find the new term. If it's not there, or if we think that this term is not the right
one, we have the old and new versions available to figure out what to do.
