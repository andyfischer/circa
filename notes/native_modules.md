
Some notes on our requirements for a native module system.

Goals:

 - Ease of writing native code
 - Live loading of shared libraries

The following workflow would be super rad:

 - Load the shared library, setup binding of native functions to Circa functions
 - Make a change to native code
 - Circa notices the source file change, initiates compiler
 - Compiler produces a shared library
 - Circa notices the new library, unloads the old shared library and loads the new one
 - New binary is executed immediately
