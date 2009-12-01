

Literal values
--------------

Circa has the following syntaxes for literal values:

 * Integer

        0, 1, -1, 1234

   Create a value of type `int`

 * Hexadecimal integer

        0x00, 0xff0000ff

   Creates a value of type `int`

 * Floating-point number

        0.0, .1, -.3, 123.456

   Creates a value of type `number`

 * String

        'hello', "goodbye"

   Creates a value of type `string`

 * Boolean

        true, false

   Creates a value of type `bool`

 * Color

        #fff, #012, #0128, #ff00ff, #00112233

   Has several different forms: 3 hex digits (corresponding to red, green, blue), 4 hex digits (same with 4th digit for alpha), 6 hex digits (red, green, blue; two digits for each color), or 8 hex digits (same, with last two digits for alpha).

   Creates a value of type `Color`, which is a list of four `number`s

Function calls
--------------

A function call has the name of the function, then `(`, then a list of arguments, then `)`. Each argument is an expression, and they are separated by spaces or commas or semicolons.

Examples:

    add(1, 2, 3)
    add(1 2 3)
    rand()
    background(#000)
    print('hi')

There's also the `->` operator for function call chaining. This takes the result of the expression on the left and uses it as an input to the function name on the right. Examples:

    'hi' -> print
    load_file() -> filter -> save

Infix operators
---------------

We have the following infix operators:

<table>
<tr>
 <th>Operator</th><th>Function name</th><th>Descrption</th><th>Precence</th>
</tr>
<tr>
 <tr><td>::</td>      <td>annotate_type</td><td>Specifies a value's type</td><td>8</td></tr>
 <tr><td>..</td>      <td>range</td><td>Range of integers</td><td>8</td></tr>
 <tr><td>*</td>      <td>mult</td><td>Multiplication (overloaded)</td><td>7</td></tr>
 <tr><td>/</td>      <td>div</td><td>Division, floating-point</td><td>7</td></tr>
 <tr><td>//</td>      <td>div_i</td><td>Division, integral</td><td>7</td></tr>
 <tr><td>%</td>      <td>remainder</td><td>Modulo, truncated division</td><td>7</td></tr>
 <tr><td>+</td>      <td>add</td><td>Addition (overloaded)</td><td>6</td></tr>
 <tr><td>-</td>      <td>sub</td><td>Subtraction (overloaded)</td><td>6</td></tr>
 <tr><td><</td>      <td>less_tdan</td><td>Less-tdan comparison</td><td>5</td></tr>
 <tr><td><=</td>      <td>less_tdan_eq</td><td>Less-tdan-or-equals comparison</td><td>5</td></tr>
 <tr><td>></td>      <td>greater_tdan</td><td>Greater-tdan comparison</td><td>5</td></tr>
 <tr><td>>=</td>      <td>greater_tdan_eq</td><td>Greater-tdan-or-equals comparison</td><td>5</td></tr>
 <tr><td>==</td>      <td>equals</td><td>Equality check</td><td>5</td></tr>
 <tr><td>!=</td>      <td>equals</td><td>Inequality check</td><td>5</td></tr>
 <tr><td>and</td>      <td>and</td><td>Logical and</td><td>4</td></tr>
 <tr><td>or</td>      <td>and</td><td>Logical or</td><td>4</td></tr>
 <tr><td>+=</td>      <td>add</td><td>Rebinding add</td><td>2</td></tr>
 <tr><td>-=</td>      <td>sub</td><td>Rebinding sub</td><td>2</td></tr>
 <tr><td>*=</td>      <td>mult</td><td>Rebinding mult</td><td>2</td></tr>
 <tr><td>/=</td>      <td>div</td><td>Rebinding div</td><td>2</td></tr>
</table>
