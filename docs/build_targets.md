# Build Targets #

The Circa build file has the recipes for two targets:

Circa command-line:
  A command-line app with very few dependencies. This app can run scripts, run
  unit tests, print out debugging info, run a REPL, and other stuff. 

Plastic:
  Includes Circa lib, along with SDL and OpenGL. This creates a window and provides
  an API for drawing stuff and doing other interactive things.

# Variants #

For each target there are three variants:

Test:
  As many assertions as possible are enabled, performance is not a concern. Includes
  such popular assertions as ca_test_assert() and the one that does a runtime type
  check on every single call. Best for running unit tests.

  Binaries: circa_t and plas_t

Debug:
  Some assertions are enabled, but performance-killing assertions are disabled. Symbols
  are enabled and optimizations are disabled. With this build you should be able to get
  decent performance, but also get decent diagnostics when something breaks.

  Binaries: circa_d and plas_d

Release:
  Assertions disabled and optimizations enabled. This build runs fast and isn't as
  helpful when diagnosing bugs.
  
  Binaries: circa_r and plas_r
