
# Overview #

"Inlined State" is the name of a language-level feature in Circa, which allows for persisted state
data to be declared inside the code where it is used. This construct is the primary method for
an author to declare persisted state.

The benefits of using inlined state are:

1. Convenience for the author
2. Introspection; the system is able to understand exactly which data is stateful.

# Syntax #

State is declared with the `state` keyword, with one of the following syntaxes:

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
frames. In the future, it's planned to also use this feature for long-term state
(values that persist
between program invocations), but this hasn't been implemented.

A simple example is:

    state int button_presses
    if button_pressed()
        button_presses += 1
    end

Every time this script is run, if `button_pressed()` is true then the `button_presses` count will go up.

# Nesting #

State can be nested in function definitions. An example:

    def my_counting_func(bool trigger) -> int
        state int count
        if trigger
            count += 1
        end
        return count
    end

    def function_that_has_three_counts()
        my_counting_func()
        my_counting_func()
        my_counting_func()
    end

If a piece of code calls a function with inlined state, that code also becomes stateful. Also,
there is a separate piece of state for each call to a stateful function. In the above example,
`function_that_has_three_counts` has three different ints in its state, one for each call.
(So, don't confuse this feature with C's static local variables).

The structure of state data can have many levels of nesting, if the function call graph is nested.
It's also possible to have a recursive function with inlined state (although this doesn't
work in the current implementation). Each recursive call will lazily initialize its state,
so the state data acts like a recursive data type.

# Encapsulation #

When writing a function call, the author might not know or care if that function
has inlined state. Or, they may know that a function is stateful, but they
might not care about the details. This has the advantage of encapsulation: an author can take
advantage of prewritten functions without worrying about the internal details of how those
functions manage their state.

# Interaction with control-flow constructs #

When there's inlined state inside a control flow construct, special things need to happen.

*for loops (bounded over a list)*

A separate instance of state data is created for every iteration of the loop.
So the following loop would have 100 different strings in its state:

    for i in 0..100
        state string a_string
    end

*unbounded loops (such as the `while` loop)*

Circa doesn't currently have any unbounded loops. If added, there would need to be
some way that it avoids unbounded memory usage when using inlined state. Most likely,
the entire loop would share one instance of state, not have a separate instance for
each iteration.

*if/else blocks*

Each branch of an if/else chain can have its own state. One feature that might be unexpected
is that when a condition is evaluated to be
false, all state in that branch is reset. This behavior might not be right
for every situation, but in practice it tends to be the most convenient thing to do.

An example: the following code keeps track of how long a button has been held down. Every time
that `button_down()` returns false, the state inside the elapsed() call is reset, so it
starts out at 0 again.

    duration_button_held = 0.0
    if button_down()
        duration_button_held = elapsed()
    end

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

elapsed() returns the total time since the function was first called, using the `time`
global variable.

    def elapsed() -> number
        state number time_started = time
        return time - time_started
    end

fade() will go from 0.0 to 1.0 over the course of `total_time` seconds.

    def fade(number total_time) -> number
        return min(elapsed() / total_time, 1.0)
    end
