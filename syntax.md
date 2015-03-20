---
title: Syntax
---

# Syntax documentation

Circa's syntax is directly influenced by Algol-family languages like Coffeescript, ES6 and Python.

At a high level, the syntax is designed with these features/goals:

 * Significant whitespace
 * Optional type annotations
 * Make it easy to work with immutable values (with the `@` operator)
 * Encourage dataflow-style coding (with the `|` operator)
 * A bit of Lisp flavor (no commas required between function arguments)

## Comments

    -- single line comment
    
    {-
      multi-
        line
          comment
    -}

## Literal values

    1       -- int
    0x123   -- int, hexadecimal syntax
    1.2     -- number (IEEE 754 float)
    'hello' -- string, single quotes
    "hello" -- string, double quotes
    true    -- boolean
    false   -- the other boolean
    null    -- the null value
    :symbol -- symbol, similar to a string but commonly used as an identifier or map key

## Function call

    add(1, 2)
    add(1 2)  -- commas between arguments are optional

## Name binding

Putting `name =` in front of an expression will give that expression a name.

    a = add(1 2)
    print(a)
    
## Infix expressions

    -- Precedence level 7
    a..b  -- range(a b), iterate across a range of integers
    
    -- Precedence level 6
    a * b -- mult(a b)
    a / b -- div(a b), aka floating-point division
    a // b -- div_i(1 b), aka integer division
    a % b -- remainder(a b)
    
    -- Precedence level 5
    a + b -- add(a b)
    a - b -- sub(a b)
    
    -- Precedence level 4
    a < b -- greater_than(a b)
    a <= b -- greater_than_eq(a b)
    a > b -- less_than(a b)
    a >= b -- less_than_eq(a b)
    a == b -- equals(a b)
    a != b -- not_equals(a b)
    
    -- Precedence level 3
    a and b -- and(a b)
    a or b -- or(a b)
    
    -- Precedence level 2
    a | b -- b(a), aka piping aka right-apply function
    
    -- Precedence level 1
    a += b -- a = a + b
    a -= b -- a = a - b
    a *= b -- a = a * b
    a /= b -- a = a / b
    
## List expression

    [1, 2, 3]
    [1 2 3]  -- commas are optional
    
Equivalent to calling `make_list`, such as: `make_list(1 2 3)`

## Map expression

    {:a => 1, :b => 2}
    
Equivalent to calling `make_map`, such as `make_map(:a 1)`

## Conditionals

    if condition
      do_something()

    if a
      something()
    elif b
      something_else()
    else
      another_thing()
    
A conditional can be used as an expression. The last expression of the inner contents is used as the overall value.

    a = if true
      1
    else
      2
    -- 'a' will now equal 1
    
## Switch blocks

    -- when no expression is after the 'switch' keyword, we pick the first 'case' expression that equals true.
    switch
      case a == 1
        print('a is 1')
      case a == 2
        print('a is 2')
      else
        print('a is something else')
        
    -- when there is an expression after 'switch', we pick the first 'case' that equals the initial value.
    switch a
      case 1
        print('a is 1')
      case 2
        print('a is 2')
      else
        print('a is something else')
        
## For-loop

    for i in [1 2 3]
      print('i = ' i)
      
    -- loops can also output a value
    out = for i in [1 2 3]
      i * 2
    -- 'out' is now equal to [2 4 6]
    
## Function definition

    def my_function()
      print('hi')
      
    my_function()  -- calls the function and prints 'hi'
      
If there's no `return`, the last expression of the function is used as the return value.

    def my_add(a, b, c)
      a + b + c
      
    sum = my_add(1 2 3)
    -- 'sum' now equals 6
    
Types can be explicit for inputs and outputs.

    def my_add(int a, int b) -> int
      a + b
      
