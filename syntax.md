# Syntax documentation

Circa's syntax is directly influenced by Algol-family languages like Coffeescript, ES6 and Python.

At a high level, the syntax is designed with these features/goals:

 * Significant whitespace
 * Optional type annotations
 * Make it easy to work with immutable values (with the `@` operator)
 * Encourage dataflow-style coding (with the `|` operator)
 * With a bit of Lisp flavor (no commas required between function arguments)

## Comments

    -- single line comment
    
    {-
      multi-
        line
          comment
    -}

## Literal values

    1       -- integer
    0x123   -- integer, hexadecimal syntax
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

    
