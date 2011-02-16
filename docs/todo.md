
# Great Big List of Circa TODOs #

# Bugs #

 - Can't use a namespace qualifier on a type name in function declaration

 - Can't comment out something inside of a literal list

 - Complex lvalues don't work.
     Doesn't work: something.field += x
     Doesn't work: something.field[i] = x

 - When reloading a branch, we need to search the whole world for references to the 
   old branch, and update those references.

 - 'do once' needs to export name bindings. (Maybe just make this syntactic sugar for if())

# Features #

 - Lambda expressions
 - Parametrized and generic types
 - Store whether a function is pure
 - LLVM compilation
 - C reproduction
