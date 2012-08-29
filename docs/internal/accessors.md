
## Accessors ##

An *accessor expression* is a chain of terms, where each term has one of the following
functions:

  get_index
  get_field
  certain method calls?

And, only the first term in the chain (called the 'head') has a name.

For example, the following syntax creates an accessor:

    a[1][2]['name']

This syntax compiles to the following terms:

    t1: a
    t2: get_index(t1, 1)
    t3: get_index(t2, 2)
    t4: get_field(t3, 'name')

## Selectors ##

A *selector* is a value that, given a compound value, will indicate a certain nested
value.

From an accessor expression, one can always construct a selector. Sometimes the selector
value is statically known and sometimes it is not known until runtime. In the above example,
the selector has the following value:

    [1, 2, 'name']

And, we can use the selector to obtain the same nested value that is obtained from the chain
of 'get' expressions.

## Nested assignment ##

An accessor expression may occur on the left side of the equals operator.

Open questions:

  How do we form a selector for a method call?
  Is a method expression part of the accessor expression?
