
Alpha 3 Release Notes
---------------------

Work is currently being done on alpha 3, so this document is WIP.

Added Type(Value) initialization syntax. Examples:
   p = Point([1 2])
or using right-apply syntax:
   p = [1 2] -> Point

Removed typecast operator (`::`) in favor of above syntax.

Function declarations use `->` instead of `::` to specify return type.
    def math(number a, number b) -> number

Added Pythonic syntax for blocks with significant indentation.
    def pythag(number a, number b) -> number:
        return sqrt(sqr(a) sqr(b))

