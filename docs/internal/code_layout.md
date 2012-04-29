
This document talks about how code is internally stored.

A piece of code is stored as a Branch. Each branch stores a list of Terms. Each Term has
a list of inputs, a function, possibly a name, and other metadata. A Term may also have
a nested Branch which can contain more Terms.

### Term ###

# Inputs #

Each Term has a list of Inputs, which are stored as references to other Terms.

The Function decides how many inputs the Term should have. If the Term has the wrong
number of inputs then that is a static error.

An input may be a NULL reference. We can ask the Function whether it allows an input to
be optional. If an input is NULL and the Function doesn't say it's optional, then that
is a static error.

We try to follow the following rules for input references. Note that some of these rules
are known to be ignored in some cases. It hasn't been decided yet whether these exceptions
will be allowed or fixed.

 - Each input term must occur earlier in the branch than the using term.
 - A term must not have itself as an input.
 - A term can only use inputs in the current branch, or in a parent branch. (Exceptions:
   This rule is currently broken for the namespace() and include() functions. These
   functions expose their contents, so outside terms can refer directly to their inner
   terms).

# Functions #

Each Term has a reference to another term as its function. It's a static error if this
reference is NULL or it doesn't point to a callable term.

# Names #

Each Term can optionally have a name binding.

