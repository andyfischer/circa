
### Inlined State ###

# Overview #

"Inlined State" is the name of a language-level feature in Circa, which allows for persisted state
data to be declared inside the code where it is used. This construct is the primary method for
an author to declare persisted state.

The benefits of using inlined state are:

1) Convenience for the author
2) Introspection; the system is able to understand exactly which data is stateful.

# Syntax #

State is declared with the 'state' keyword, with one of the following syntaxes:

    state <name>
    state <name> = <initial value expression>
    state <type> <name>
    state <type> <name> = <initial value expression>

Examples:

    state a
    state height = get_current_height()
    state List people
    state List people = ['Adam']

A type can be provided. If no type or initial value is provided, then the type is 'any'. If there
is an initial value and no declared type then the type is inferred. In either case, if no initial
value is provided, then the value starts off with the type's default value.

# Behavior #

Inlined state is mainly for preserving *short-term* state: values that persist between
frames. It's planned to also use this construct for long-term state (values that persist
between program invocations), but this hasn't been implemented.

A simple example is:

    state int button_presses
    if button_pressed()
        button_presses += 1
    end

Every time this script is run (in Circa world this is probably once per frame), if button_pressed() is true then the button_presses count will go up.

# Nesting #

Inlined state really begins to be useful when it's nested inside function definitions. A simple
example is:

    def my_counting_func(bool trigger) -> int
        state int count
        if trigger
            count += 1
        end
        return count
    end

When this function is used, there is a separate 'count' for *every* different call to this function.
(So, don't confuse this feature with C's static local variables). If a function includes inlined
state, then any function which has a call to this function also has inlined state, and so on,
all the way up the call stack. For any block of code, the Circa compiler knows the data type of its inlined state (if any).

This does not mean that recursion is impossible. Inlined state is lazily created, so a recursive
function with inlined state would have a recursive data type.

# Encapsulation #

Since inlined state is builtin to the language, an author might not know or care if the functions
they use have inlined state. Or they may know that some code has state, but they
might not care about the details. This has the advantage of encapsulation: the author can take
advantage of prewritten functions, without worrying about the internal details of how those
functions manage their state. This style of programming feels similar to object-oriented
programming, but it tends to be simpler.

# Helpful stateful functions #

Here are some examples of simple helper functions that take advantage of inlined state.

toggle() will switch its result every time it's called with 'true':

    def toggle(bool switch) -> bool
        state bool status
        if switch
            status = not(status)
        end
        return status
    end

approach() will return a result that approaches a target over time:

    def approach(number target, number maximum_change) -> number
        state current = target
        if target > current
            current += min(step, target - current)
        elif target < current
            current -= min(step, current - target)
        end
        return current
    end

elapsed() returns the total time since the function was first called, using the 'time'
global variable.

    def elapsed() -> number
        state number time_started = time
        return time - time_started
    end

fade() will go from 0.0 to 1.0 over the course of 'total_time' seconds.

    def fade(number total_time) -> number
        return min(elapsed() / total_time, 1.0)
    end

# Interaction with control-flow constructs #

When there's inlined state inside a control flow construct, special things need to happen.

for loops (bounded over a list)

A separate copy of state data is created for every iteration of the loop.
So the following loop would have 100 different strings in its state:

    for i in 0..100
        state string a_string
    end

unbounded loops (such as the 'while' loop)

Circa doesn't currently have any unbounded loops, but if added, there would need to be
some way that it avoids unbounded memory usage when using inlined state. The most
likely solution is that the loop would only use one instance of its state, not a
separate instance for each iteration as with the for-loop.

if/else blocks:

Each branch of an if/else chain can have its own state. One feature that might be unexpected,
is that when a condition is evaluated to be
false, all state in that branch is reset to defaults. This behavior might not be correct
for every situation, but in practice it tends to be the most convenient thing to do. It was
a design decision to make if-blocks always behave a certain way, rather than let the author specify
whether if-block state should be reset. If the author doesn't want state to be reset, there
are easy ways to write the code differently (such as by declaring the state variable outside
of the if-block).

An example: the following code keeps track of how long a button has been held down. Every time
that 'button_pressed()' returns false, the state inside the elapsed() call is reset, so it
starts out at 0 again.

    duration_button_held = 0.0
    if button_pressed()
        duration_button_held = elapsed()
    end
