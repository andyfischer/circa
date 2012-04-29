
# Outline of interpreter #

Each interpreter is stored as a first-class object called a Stack. The program can have
several Stacks at once.

A Stack has a list of Frames, each one acts as an activation record for a certain Branch.

The Frame contains:
  A reference to a Branch
  A list of 'register' values
  A current PC and a next PC

The Frame's registers are the "local" values. Each one corresponds to a Term, and it's
easy to go from Term to register or vice-versa.

The topmost Frame is the one currently being evaluated.

# STARTING THE INTERPRETER #

There is some code that invokes the interpreter (calling the run_interpreter function),
we refer to this as the "caller". When starting an interpreter session, the caller will
manually push a branch onto the stack (creating a frame). Then, the caller may copy input
values to the frame's registers. The caller should copy an input for each
input_placeholder() term.

The caller executes run_interpreter(). We refer to an "interpreter session", which is the
duration of the run_interpreter function. When that function exits, the "session" has
ended.

During evaluation, the interpreter will evaluate each term in sequence, and save results
to the frame registers. The stack may grow and shrink with control flow operations.

The interpreter session terminates either when the end of the topmost branch is reached,
or the interpreter encounters an error.

If the interpreter finishes the topmost branch, it will exit the session and leave the
topmost frame as is. The caller should read the output values (if any) from this frame
and then clear the stack.

If there is an error, the stack will be left in its exact state at the time of the error.
The caller should check for an error once the session ends, and handle the error
appropriately (such as displaying it to the user). The caller must expect that the stack
may have more frames than expected.

# THE INTERPRETER LOOP #

The interpreter loop goes as follows:
 
 1: Advance PC to nextPC, and set nextPC to (PC + 1)
 2: Check if we have started a Branch that has an override func. If so:
       Set nextPC to the last index in this branch
          Execute the C override
          If the C override raised an error:
             Terminate
          Repeat loop
 3: Look at PC. If we have reached the end of the top frame, then:
       If this is the topmost frame:
          Terminate the session
       Otherwise:
          Copy outputs to above frame
          Pop the topmost frame
          Repeat loop
 4: Look at the Term at the current PC.
 5: If the Term has no evaluation func, or the Branch is emptyEvaluation:
       Repeat loop
 6: Choose the Branch to push. The selection of the Branch depends on the term's function
       For an if-block or switch, the Branch will be selected based on the condition or
         input value
       Otherwise, if the term has a nested branch, use that.
       Otherwise, use the branch corresponding to the term's function
 7: Start a new frame
       Push a frame using the chosen branch
       Copy input values to the new frame. Check that values match expected types.
       If an input doesn't fit the Branch's expected type:
           Raise error and terminate
       Repeat loop

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
