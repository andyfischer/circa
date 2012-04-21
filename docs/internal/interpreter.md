
Outline of how the interpreter runs.

Each interpreter is stored as a first-class object called a Stack. The program can have several Stacks at once.

A Stack has a list of Frames. A Frame is an activation record.

Each Frame has a Branch and a list of "registers" (a list of Values). A frame also has a PC (program counter) index, which is an index into its Branch. The PC index may be past the last term, in which case the frame is about to finish.

A Branch is an object that stores compiled code. It does not store data associated with one invocation (usually). A Branch contains a list of Terms.

The Frame's registers are the "local" values. Each one corresponds to a Term, and it's easy to go from Term to register or vice-versa.

The topmost Frame is the one currently being evaluated.

# STARTING THE INTERPRETER #

The run_interpreter function runs an interpreter until interrupted or finished. This function takes a Stack as input, and this stack must have at least one Frame on it.

When starting an interpreter run, the calling code will manually push a branch onto the stack (creating a frame). Then, the calling code will copy input values to the frame's registers. Generally, the calling code should copy an input for every input_placeholder() term.

Once run_interpreter starts, it remembers the original "top" frame. This frame will not be popped, instead if we reach the end of this frame, the run_interpreter function will return. During the course of evaluation, we might push additional frames. And, if an error or interrupt occurs, we might return from run_interpreter with more frames than we started with. But not less.

When finished, the calling code will check to see if there is an error. The caller may examine the error, and they may dispose of it. After making sure to throw away any extra frames that it doesn't care about, the caller may fetch output values from the top frame's registers. Generally, it should fetch the values for each output_placeholder() term.

The calling code may then decide to reset the PC if it wants to repeat the same frame, or it may toss out the frame, or it may be finished with the entire Stack.

# THE INTERPRETER LOOP #

The interpreter loop goes as follows:

 1: Check if this branch is overridden by a C function
 2: Look at PC. If we have reached the end of the frame, then go to finish_frame
 3: Look at the Term at the current PC.
 4: If the Term has no evaluation func, do nothing and goto next.
 5: Fetch the Branch to push. The selection of the Branch depends on the term's function
     - For an if-block or switch, the Branch will be selected based on the condition or
       input value
     - Otherwise, if the term has a nested branch, use that
     - Otherwise, use the branch corresponding to the term's function
 6: Push a frame for this branch
   - Check if this new branch is overridden by a C function
 next:
   - Advance PC by one
   - Return to step 2

# BRANCH EVALUATION #

When pushing a new branch to the stack, there are promises made:

  - Caller promises to put input values in the registers for each input_placeholder. This
      might be done by the interpreter or it might be external code.

  - Callee promises to do all the actions that the branch normally promises
  - Callee promises to leave output values in the registers for each output_placeholder

# BRANCH OVERRIDING #

Any branch can be overridden by a foreign function. When this happens, the interpreter
doesn't step through the Terms for the branch. Instead, the override function is called
once, and it's expected to fulfill all the promises mentioned above.

# ACCESSING THE STACK #

These functions are intended to be used by evaluation override functions.

    circa_input(stack, index)

        Fetches the input value passed by caller

        Impl: Fetches the nth register on the top frame, as long as this register
              corresponds with an input_placeholder

    circa_num_inputs(stack)

        Counts the number of input placeholders in the top frame's branch

    circa_output(stack, index)

        Fetches the output value container, to be returned to the caller

        Impl: Fetches the nth-from-the-end register on the top frame, as long as this
              register corresponds with an output_placeholder

    circa_caller_term(stack)
        
        Returns the term that was used for this call. May return NULL if the branch
        was pushed manually.

        Impl: Returns the current term (according to PC) in the second-to-top frame.

    circa_caller_input_term(stack, index)

        Fetches the nth input Term that was used for this call. If the branch
        was pushed manually, there might be no such term, in which case this
        will return NULL.

        Impl: Looking at the 'current' term in the second-to-top frame, this will return
                the nth input term of this.

    circa_caller_branch(stack)

        Promise: Returns the branch where the current call was made.
        Impl: Returns the branch of the second-to-top frame.








# EXAMPLE #

We'll use two branches:

BranchA: {
    input0
    x = input0 + 1
    y = b(x)
    z = y + 1
    output0 = z
}

BranchB: {
    b_input0
    b_result = b_input0 * 2
    b_output0 = b_result
}

Caller starts by manually pushing BranchA to the stack. Our stack looks like:

Frame 0:
> 'input0' unevaluated
  'x'  unevaluated
  'y'  unevaluated
  'z'  unevaluated

The > on the left points to the current PC.

At this point, here is what the accessors will return:

    circa_input(stack, 0)      - returns the value for input0
    circa_input(stack, 1)      - returns NULL, there's no input 1
    circa_outut(stack, 0)      - returns the value for output0
    circa_num_inputs(stack)    - returns 1
    circa_caller_term(stack)   - returns NULL, there's no caller branch
    circa_caller_input_term    - returns NULL
    circa_caller_branch(stack) - returns NULL

Caller uses circa_input to access input0, and copies '5' to it. Stack looks like:

Frame 0:
> 'input0' 5
  'x'  unevaluated
  'y'  unevaluated
  'z'  unevaluated

Caller then starts the interpreter. Interpreter evaluates each call.

Here is what the interpreter looks like when it starts evaluating 'y' (which calls
the function b())

Frame 0:
  'input0' 5
  'x'  6
> 'y'  unevaluated
  'z'  unevaluated
Frame 1:
> 'b_input0'  5
  'b_result'  unevaluated
  'b_output0' unevaluated

The interpreter has pushed a frame with branch B, and copied y's input value (5) to the
input register.

At this point, here is what the accessors will return:

    circa_input(stack, 0)      - returns the value for b_input0
    circa_input(stack, 1)      - returns NULL, there's no input 1
    circa_output(stack, 0)     - returns the value for b_output0
    circa_num_inputs(stack)    - returns 1
    circa_caller_term(stack)   - returns the 'y' term
    circa_caller_input_term(stack, 0) - returns the 'x' term
    circa_caller_branch(stack) - returns BranchA

Temp notes on refactoring:

circa_input_term -> circa_caller_input_term
circa_frame_input -> circa_input
circa_frame_output -> circa_output
circa_current_term -> circa_caller_term
circa_callee_branch -> circa_caller_branch
