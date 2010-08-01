
Circa's object model uses a pluggable type system and tagged pointers. A type is defined
by creating a Type object, and adding functions to its slots to control its behavior.
The type system is meant to be pluggable and flat. There's no (currently) no inheritance
or type hierarchies.

Values are held by TaggedValue objects, each TaggedValue has a pointer to the type
used, and a word-sized slot (called `value_data`) where the actual value is held.
The type can use this space
to hold the actual value, or it can hold a pointer to an allocated object.

# Type slots #

Each Type object has the following slots for controlling behavior. Here's a summary on
how each is used:

## initialize ##

This is called immediately after a TaggedValue is changed to the given type. `value_data`
is always initialized to 0 before initialize(). This function is optional.

## release ##

This is called immediately before a TaggedValue is destroyed or changed to hold a different
type.

## copy ##

Called when a value of this type should be copied to another TaggedValue. This copy should
behave like a separate value, not a reference to the same value. (if one value is later
changed then that change shouldn't affect the copy). But, if the type is able to guarantee
immutability then it can always reuse the same data on a copy.

If this function isn't implemented, the default behavior is to do a shallow copy of
`value_data`

## reset ##

Reset an existing value to its 'initial' value. This should have the same effect as
'initialize'.

If this function isn't implemented, the default behavior is to call release() and then
initialize().

## cast ##

Attempt to convert a value of this type to a value of a different type. The type can assume
that this will only be called if cast_possible returns true;

If this function isn't implemented, the default behavior is to do a copy()

## castPossible ##

Returns whether it's possible to convert a value of this type to the given type.

If this function isn't implemented, the default behavior is to always say no.

## equals ##

Returns whether the given values are equal. The right-hand-side value might be of a
different type, so this function should check the types.

If this function isn't implemented, the default behavior is: when the values have
the same type, do a shallow comparison of `value_data`, and if they have different
types, always return false.

## toString ##

Return a human-readable string describing this value. This is mostly used for debug
output.

## formatSource ##

Returns the value as a snippet of Circa source code. This is used for values which can
be typed as literals in source code.

## checkInvariants ##

Not used yet

## touch ##

This is called when the contents of this value are about to be modified. Types which
use copy-on-write data sharing should create a real copy of their data here.

## getIndex ##

Access a value by index. This is used for lists or list-like objects.

## setIndex ##

Set a value by index.

## getField ##

Access a value by name. This is used for dictionary-like objects.

## setField ##

Set a value by name.

## remapPointers ##

Deprecated.
This function is called when a Branch is being
duplicated, and each local Term reference needs to be remapped from the old branch to the
new copy. If the type holds any Term reference, then they should remmap them (according
to the provided map).

