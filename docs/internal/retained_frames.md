
## Retained Frames ##

Written on 9/4/2013

#### OVERVIEW #####

This document details Circa's design of retained frames. By "frames" we mean stack frames,
(also called activation frames) which are usually discarded during execution. A stack frame
that is "retained" is preserved across successive executions. This creates a tree-like data
structure which is a subset of the call graph.

#### MOTIVATION ####

The retained frame system was originally added to implement inlined state (which is the primary
way that Circa programs store state). Additionally, retained frames are used for call-site-based
memoization, which exists as a user-directed optimization.

Having retained frames adds overhead to the interpreter, but we think there is a good tradeoff,
because they also enable some of Circa's more interesting and distinctive features. So, for
better or worse, we're embracing the retained-frame strategy.

Below are the gorey details. Note that it might be easiest to skim the "data structures" section
and then jump straight to "examples".

#### DATA STRUCTURES ####

On the Frame structure are two relevant fields:

    List state
    bool shouldRetain

'state' is either null or a List, where each element corresponds to the Term with the same index.

Each 'state' element can be one of:

 - null
 - a RetainedFrame value
 - a List of RetainedFrame values

The RetainedFrame type is a subset of Frame, and has the following fields:

    Stack stack
    Block block
    List state
    List registers 
 
(Implementation note: RetainedFrame is created as a hosted type, whereas Frame is a native type).

#### RUNTIME ####

##### When a normal function is called #####

On every normal function call, the calling frame's `state` is checked to see if is non-null,
and whether there is a
RetainedFrame value for the current term index. If so, the retained frame's `state` and
`registers` (if non-null) are copied to the new frame.

##### When an indexed function is called #####

By "indexed" we mean a for-loop, where each iteration has a separate index, or an if-block,
where each condition block has a separate index.

When one of these functions is called, we check to see if the calling frame's `state` is
non-null, and we look at the state element that corresponds to the current term index.
We then check to see if this state element is a List (instead of a RetainedFrame), and
then we pick out the element corresponding to the iteration index (for-loop)
or condition index (if-block). This element will (hopefully) be a RetainedFrame,
which is used to populate the new frame.

#### EXAMPLES ####

##### State in a normal function call #####

With code:

    state a = 'value'

The finished stack looks like:

frames:
 frame 0: Frame {
  state: List {
   0: null,
   1: null,
   2: 'value'
  }
 }
     
With code:

    def f()
        state a = 'value'

    f()

The finished stack looks like:

frames:
 frame 0: Frame {
  state: List {
   0: null,
   1: RetainedFrame {
     state: List {
      0: null,
      1: null,
      2: 'value',
     registers: null
    }
   }
  }
 }

##### Memoizing a normal function call #####

With code:

    def f(int i) -> int
      memoize()
      return i + 1

    f(3)

The finished stack looks like:

frames:
 frame 0: Frame {
  state: List {
   0: null,
   1: RetainedFrame {
     state: null,
     registers: List {
       0: 3
       1: null
       2: null
       3: 4
     }
    }
   }
  }
 }
 

##### State in a for-loop #####

With this code:

for i in [0 1 2]
    state s = i

The finished stack looks like:

frames:
 frame 0: Frame {
  state: List {
   0: null,
   1: List {
     0: RetainedFrame { state: List { 0: null, 1: 0 }, registers: null }
     1: RetainedFrame { state: List { 0: null, 1: 1 }, registers: null }
     2: RetainedFrame { state: List { 0: null, 1: 2 }, registers: null }
   }
  }
 }
