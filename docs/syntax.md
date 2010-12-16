
Introduction
------------

This page is a reference for Circa's syntax.

Comments
--------

Line comments are specified with two dashes: `--`. This will cause the rest of the line to be ignored.

    -- this is a comment

Currently there's no syntax for block comments, this is planned for the future.

Integers
--------

Examples:

    0, 1, -1, 1234

There is also hexadecimal syntax:

    0x00, 0xff0000ff

These create a value of type `int`.

Floating-point numbers
----------------------

Examples:

    0.0, .1, -.3, 123.456

These create a value of type `number`.

Exponent syntax is currently not supported (example: `1e9`), but planned for the future.

Strings
-------

Strings can either be single-quoted or double-quoted.

    'hello', "goodbye"

These create a value of type `string`.

Booleans
--------

Values of type `bool` have two possible values:

    true, false


Colors
------

Since Circa scripts tend to draw stuff, the language supports color literals. The syntax is similar to CSS.

Examples:

 * `#fff` (white)
 * `#f0f` (magenta)
 * `#00f8` (blue with 50% transparency)
 * `#ff4f00` (International orange)
 * `#00ff0088` (Green with 50% transparency)

The different formats are:

 * 3 hex digits (corresponding to red, green, blue)
 * 4 hex digits (as above, with 4th digit for alpha)
 * 6 hex digits (red, green, blue; two digits for each color)
 * 8 hex digits (as above, with last two digits for alpha).

Creates a value of type `Color`, which is stored internally list of four numbers.

Name bindings
-------------

To use a result somewhere else, one must bind it to a name. A name binding statement is of the form:

    <identifier> = <expression>

Some examples:

    one = 1
    four = 2 + 2
    output = concat('There are ' number_of_things ' things)

Function calls
--------------

A function call has the name of the function, then `(`, then a list of arguments, then `)`. Each argument is an expression, and they are separated by spaces or commas or semicolons.

Examples:

    add(1, 2, 3)
    add(1 2 3)
    rand()
    background(#000)
    print('hi')

Right-apply syntax
------------------

The `->` operator is shorthand, it calls the function on the right using the value on the left as input.

This code sample:

    'hi' -> print

does exactly the same thing as:

    print('hi')

(Currently this syntax only supports sending a single output value to a function which takes a single input. In the future we'll support the same syntax for multiple inputs.)

Typecasting
-----------

A type name can be called like a function, this will attempt to cast the input to the specified type.
    a = number(1)
    print(a)     -- prints 1.0

Since this is just a function call, you can also use the right-apply operator to do the same thing (`->`):

    a = 1 -> number

Unary expressions
-----------------

Circa has the following unary operators:

<table>
<tr>
 <th>Operator</th><th>Function name</th><th>Descrption</th>
</tr>
<tr><td>-</td>      <td>neg</td><td>Unary negation</td></tr>
<tr><td>@</td>      <td>(none)</td><td>Rebinding operator</td></tr>
</table>

A note of warning when using unary negation: if you have a list of items separated by spaces, sometimes whitespace will affect parsing. Examples:

 * `[3 - 2]` - creates a list of one element, equal to `[1]`
 * `[3 -2]` - creates a list of two elements, equal to `[3, -2]`
 * `[3-2]` - creates a list of one element, equal to `[1]`

The rebinding operator `@` is explained below.

Infix expressions
-----------------

Circa has the following infix operators:

<table>
<tr>
 <th>Operator</th><th>Function name</th><th>Descrption</th><th>Precedence</th>
</tr>
<tr>
 <tr><td>.</td>      <td>(multiple)</td><td>Named field access or member function call</td><td>9</td></tr>
 <tr><td>-></td>      <td>(none)</td><td>Right-apply</td><td>8</td></tr>
 <tr><td>..</td>      <td>range</td><td>Range of integers</td><td>8</td></tr>
 <tr><td>*</td>      <td>mult</td><td>Multiplication (overloaded)</td><td>7</td></tr>
 <tr><td>/</td>      <td>div</td><td>Division, floating-point</td><td>7</td></tr>
 <tr><td>//</td>      <td>div_i</td><td>Division, integral</td><td>7</td></tr>
 <tr><td>%</td>      <td>remainder</td><td>Modulo, truncated division</td><td>7</td></tr>
 <tr><td>+</td>      <td>add</td><td>Addition (overloaded)</td><td>6</td></tr>
 <tr><td>-</td>      <td>sub</td><td>Subtraction (overloaded)</td><td>6</td></tr>
 <tr><td><</td>      <td>less_than</td><td>Less-than comparison</td><td>4</td></tr>
 <tr><td><=</td>      <td>less_than_eq</td><td>Less-than-or-equals comparison</td><td>4</td></tr>
 <tr><td>></td>      <td>greater_than</td><td>Greater-than comparison</td><td>4</td></tr>
 <tr><td>>=</td>      <td>greater_than_eq</td><td>Greater-than-or-equals comparison</td><td>4</td></tr>
 <tr><td>==</td>      <td>equals</td><td>Equality check</td><td>4</td></tr>
 <tr><td>!=</td>      <td>equals</td><td>Inequality check</td><td>4</td></tr>
 <tr><td>and</td>      <td>and</td><td>Logical and</td><td>3</td></tr>
 <tr><td>or</td>      <td>and</td><td>Logical or</td><td>2</td></tr>
 <tr><td>+=</td>      <td>add</td><td>Rebinding add</td><td>2</td></tr>
 <tr><td>-=</td>      <td>sub</td><td>Rebinding sub</td><td>2</td></tr>
 <tr><td>*=</td>      <td>mult</td><td>Rebinding mult</td><td>2</td></tr>
 <tr><td>/=</td>      <td>div</td><td>Rebinding div</td><td>2</td></tr>
</table>

The `/` operator always does floating-point division (unlike C where it can sometimes do integer division). To do integer division, use the `//` operator.

Each 'rebinding' operator will perform the advertised operation, and then rebind the left-hand-side name to the result. Example:
    
    a += 1

Does the same thing as:
   
    a = a + 1


Rebind operator
------------------

The `@` operator is used for shorthand, it means that the given name should be rebound to the result of the whole expression.

So instead of typing 'a' twice in this example:

    a = add(a, 2)

You can write this, for the same effect:

    add(@a, 2)

Lists
-----

Lists are specified with the `[` and `]` symbols. The elements of a list can be separated by spaces or commas or semicolons (just like function call arguments).

    []
    [1 2 3]
    ['apple', 'banana', 'cucumber']

Indexed access
------------

The `[]` symbols are used to access an element in a list by index. Indices are zero-based.

    example_list = [1 2 3]
    example_list[0]     -- equals 1
    example_list[1]     -- equals 2
    example_list[2]     -- equals 3

Note that there must not be any whitespace between the expression on the left and the `[` symbol, otherwise the right side will be parsed as a list.

    print(example_list[0])   -- prints '1'
    print(example_list [0])  -- prints '[1 2 3][0]'

Function declarations
---------------------

A new function can be declared with the `def` keyword. Example:

    def <function-name>(<list of arguments>) -> <optional-return-type>
        <statements>

An example:

    def print_sum(number a, number b)
        sum = a + b
        print(sum)

The indentation is significant. Lines which are indented to the left of the 'def' statement are considered to be inside the subroutine.

A function can also be a one-liner, if there are any expressions to the left of the declaraion. The 'end' keyword is used to mark the end of the function.

    def one_liner(number a, number b) sum = a + b; print(sum) end

A function can return a value, if it declares a return type. This is done with a -> token at the end of the declaration.

    def pythag(number a, number b) -> number
        return sqrt(a*a + b*b)

Once a function is declared, it can then be called like a normal function.

    pythag(3, 4)           -- returns 5
    print_warning('hi')    -- prints 'warning: hi'

If statement
------------

An if-statement is specified by the `if` keyword, followed by a conditional expression, followed by a list of statements. The format is: 

    if <condition>
        <statements>
        
The inner statements are executed if the conditional expression is true. An example:

    if mouse_pressed()
        print('The mouse is pressed')

Like functions, indentation is used to tell which statements are inside the block. One-liners are also possible:

    if a > b; print('a is higher'); end

Statements can be separated by line breaks, commas, or semicolons, but they are optional. The above line would work fine if it looked like this:

    if a > b print('a is higher') end

An if-statement can also have an `else` section, which is evaluated if the conditional expression is false. Examples:

    if a > b
        highest = a
    else
        highest = b

There is also the `elif` keyword, which is followed by another conditional expression. There can be many of these.

    if a > b
        print('a is higher')
    elif b > a
        print('b is higher')
    else
        print('a equals b')

For loop
-------------

A for-loop is specified by the `for` keyword. The loop has this syntax:

    for <iterator name> in <list expression>
        <statements>

An example:

    for i in [1, 2, 3, 4, 5]
        print('i is ' i)

In that example, the list will go through each number from 1 to 5, and print out something like 'i is 1' or 'i is 3'. The 'i' name is called the iterator. When executing the list, the iterator is bound to each value in the list, one at a time.

This piece of code:

    for i in [0 1 2 3 4]
        print(i ' squared is ' i*i)

has the following output:

    0 squared is 0
    1 squared is 1
    2 squared is 4
    3 squared is 9
    4 squared is 16

When iterating across a range of numbers, it's convenient to use the range operator (`..`). The range operator takes two integers and outputs a list of each integer within that range. The left input is included and the right input is excluded. Example:

    a = 0..4
    -- a is now equal to: [0 1 2 3]

So, the above for-loop can be rewritten as:

    for i in 0..5
        print(i ' squared is ' i*i)

For loop with rebinding
----------------------------

A for-loop can also modify the list that it iterates through. To do this, use the @ operator in front of the list name, to indicate that it will be rebound to a new value. Then in the body of the loop, rebind the iterator to a new value. The loop will collect the results of each iteration, and use those results as the new value of the list.

Here's an example. We start with the numbers 0 though 4:

    numbers = [0 1 2 3 4]

In the for loop, we use the @ operator to say that we are going to rebind 'numbers'

    for i in @numbers
        i = i*i

Inside the loop, we rebind 'i' to the square of the iterator.

    print(numbers)   -- outputs: [0, 1, 4, 9, 16]

The end result is that each item in the list is squared.

Another way that the list can be modified is with the `discard` statement. If the loop encounters a `discard`, then it will remove the current element fron the result list.

Here's an example. This list iterates through each number between 0 and 9. Each time it finds that the number is odd, it triggers `discard`:

    numbers = 0..10
    -- numbers is now equal to [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
    for i in @numbers 
        if i % 2 == 1
            discard

    -- numbers is now equal to [0, 2, 4, 6, 8]

The result is that only the even numbers remain.

Compound types
--------------

Compound types can be declared using the `type` keyword, and providing a list of typed fields inside brackets. An example:

    type Complex {
        number real
        number imag
    }

Fields can be seperated by commas, semicolons, or newlines:

    type Triangle { Point a, Point b, Point c }

A compound value can then be created using constructor syntax, or by taking an existing compound value and casting it.

    complex_value = Complex()
    complex_value = [1.0 0.0] -> Complex

Circa currently doesn't support constructors with arguments (such as `Complex(1.0 0.0)`), but this is planned for a future version.

Fields of a compound value can be accessed or assigned with the dot operator. Example:

    real_part = complex_value.real
    complex_value.imag = 1.0

Namespaces
----------

Code can be organized into a namespace. The syntax is:

    namespace <name>
        <statements

Like other blocks, indentation is used to tell what belongs inside the block.

Example:

    namespace web
        def get_page(string url) :: string
            ...

To access something inside a namespace, use a colon-seperated identifier. Here's how we would call the 'get_page' function within the 'web' namespace:

    contents = web:get_page(url)
