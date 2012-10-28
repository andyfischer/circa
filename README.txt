
Introduction
============

Circa is a programming language designed for live coding. Most programming languages
try to improve *what* you can express, with Circa we instead try to improve *how* you
create code.

Features
========

 - Code can be modified at runtime
 - State can be redefined at runtime, and is preserved across code changes
 - Changes can be made by any external program that can modify text files
 - Simple API for making code changes, enabling non-text methods for modifying code
 - Supports a round trip where compiled code can be unparsed back to source text,
   allowing runtime changes to be saved as text files.
 - Supports plugins (which themselves can be recompiled and reloaded at runtime)
 - Embeddable in a C/C++/Obj-C application

Implementation
==============

Some implementation details:

 - Implemented in C++
 - Interpreted execution. (some form of JIT is planned for the future)
 - Compiled code is stored in a normalized, unoptimized format
 - Static typing with a pervasive 'any' type. Supports simple type inferrence.
 - Cross-platform, primary platform is OSX

Current status
==============

This project is currently alpha-level. Curious people, or people interested in
hacking along, are welcome to try it out. People who want to use it for serious
projects, or people who don't like bugs, are recommended to wait for a beta release.

Building
========

To build everything, just run "make"

For more information, see BUILDING.txt .

Directories
===========

 /3rdparty - Code imported from other projects.
 /build    - Holds all build artifacts. Can be deleted for a clean build.
 /docs     - Documentation.
 /extras   - Files relevant to specific platforms or tools.
 /improv   - Root directory for Improv app, a graphical tool.
 /include  - Public include files, this is the directory to add to your include dirs.
 /libs     - Optional Circa modules, such as bindings to 3rd-party libraries.
 /notes    - Unorganized notes and plan files.
 /old      - Deprecated code that isn't ready to be deleted.
 /src      - Circa source code.
 /tests    - Script-based tests. See also src/unit_tests for C++ based tests.
 /tools    - Scripts for Circa developers.

Documentation
=============

Refer to the /docs folder.

License
=======

Unless otherwise specified, the contents of this project are freely available to the
public under the MIT licence. See the LICENSE file for more information.
