
Some thoughts on using state machines as a core element of Circa.

State machines have many appealing properties that make them a good
fit for Circa:

 * Flexible and powerful. 
 * Easy to introspect and visualize
 * Easy to reason about, one can understand their behavior without executing anything
 * Can be created or modified declaratively

Questions..

How does the user specify behavior? (probably with feedback)

How is the structure stored?

How is the FSM executed?

--

Data structure notes:

Things that users need:
 * Refer to states by name
 * Check if we are in a certain state
 * Ask what state we are in

Normal execution:
 * Every state has an 'iterate' method
 * Call 'iterate'
 * 'iterate' might change our state
 * Then we call 'iterate' again on the new state

Traditional state machine has 'edges'. Edges can be:
 * Pieces of input, such as an RE
 * Conditions
   * Problem: multiple conditions might be true, then what do we do?

So there are different styles of state machines. Maybe we need to support multiple kinds.

Condition-based scheme could be translated into an input-based scheme.
Input-based scheme could be turned into conditions.
But which should we use?  Maybe allow both?
