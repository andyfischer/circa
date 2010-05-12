
This document has a few ideas for various kinds of introspection that we could do on Circa code.

# Execution condition #

For any term, one could form an expression that tells whether that term is executed or
not.

This represents the condition of the enclosing if-statement (if any)
Or all the if statements.
Usually this will just be True

Example:

if a:
  if b:
    c = f()

The execution condition of `c` would be and(a,b)

How to solve this in a for-loop?
How to solve this if `return`, `break`, or `continue` are added?
Also need to know if an error would occur before that term

# Why did an expression give a result #

Example, if I had this code:

def some_bool_func(bool a, bool b, bool c) -> bool
  return a and (b or c)
end

Then let's say I called the function like this:

some_bool_func(false, true, false)

And then it returns false. I want to be able to ask the system, hey, why did you return
false instead of true? In this case, it could just tell me that it's because I called it
with a=false.

In the real world, code is going to be much more complicated. But I think that there are
still going to be situations where there are very easy answers to useful questions.
Circa is all about transparency and introspection, so this is something we can do good at.

### What kinds of questions can we support? ###

*Why did you return X:* This question is not useful; the reason we returned X is because
the code is exactly the way it is. Some related useful inquiries could be: show me the
control flow used for these inputs, or show me the dataflow parents for one value.
For booleans, this question implies 'Why didn't you return X', see below.

*Why didn't you return X:* This question is more useful, we can sometimes search and find
ways that a piece of code could have resulted in Y. This is helpful for booleans, having
only two values can restrict the search space. This is also useful when dealing with a
'switch' style block, or a map access, since it's easy to figure out what input would return
a certain output.

*Why did (or didn't) you execute this term?* - To answer this one, need to know:
 1. if there were any enclosing if() statements, why was their condition false?
 2. did an error occur?
 3. was a break/return statement triggered?

'When does this return X' - Similar to the why questions, but with no context information.
This just attempts to provide a generic view on code. See the Nesting section below.

### Recursion ###

Most queries will need to be recursive: asking question Q1 on term T1 may cause that to ask
question Q2 on T2, etc.

### Complexity limit ###

A query should be able to specify a complexity limit, because some questions can just spiral
out of control. Perhaps infinitely. In the real world, I think that only simple answers are
going to be useful. If an answer grows to be too complex, might as well throw it out and
just tell the user "It's complicated". Or perhaps, offer a high-level answer and
allow the user to dig in
to the aspects that are useful to them.

### Nesting ###

Would be a good approach to just support question nesting from the get-go.

Imagine a system that is really lazy. I have this code:

    X = or(A,B)

and I click "When is this be true"?  The lazy bastard response would be to say:

    X would be true when both of these are satisfied:
        A is true
        B is true

Then maybe I can click on those terms. I click on "When would A be true". The view expands
to something else:

    X would be true when both of these are satisfied:
        A is true, this happens when:
            Y is equal to 5
        B is true

Then I keep clicking:

    X would be true when both of these are satisfied:
        A is true, this happens when:
            Y is equal to 5, this happens when:
                mouse.x returns 5
        B is true

All of this is possible without context, it would just be an inverted (result-centric)
view of the code.

 - Another useful button would be to 'zoom', to abandon some outer levels and focus on
an inner layer.

