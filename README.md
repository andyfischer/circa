
Hiatus Notice - Nov 2015
==============

Hi all. This project has had extremely slow development for the past year, so I'm calling
it, it's officially on hiatus. With two darling kids in my life, it's tough to
find the time for such an ambitious project.

I will probably take another stab at building a system like this, maybe a few years from now,
and I think most of this codebase will be rewritten with lessons learned.

Thanks for following and keep coding,

-andyf

Introduction
============

Circa is a programming language designed for live coding. Most programming languages
focus on improving *what* you can express, with Circa we focus on improving *how* you
create code.

Features
========

 - Static typing with pervasive 'any' type.
 - Immutable values by default, backed by persistent data structures.
 - Simple type inference.
 - Supports closures.
 - Builtin types include: lists, maps, strings, colors, points, rectangles.
 - Memory model that avoids global garbage collection
 - Clean syntax resembling Python or Ruby.
 - Has a builtin module system.
 - Code can be inspected and modified at runtime.
 - State is a first-class entity, and is declared inline with code.
 - Has best-effort support for state migration across arbitrary code changes.
 - Builtin support for various methods of live modification, including automatic
   reloading of source text files.
 - Simple C API, supports extending and embedding.

Implementation
==============

 - Implemented in portable C++.
 - Small number of external dependencies.
 - Cross-platform. Primary platform is OSX.

Current status
==============

This project is currently alpha-level. Curious people, or people interested in
hacking along, are welcome to try it out. People who want to use it for serious
projects, or people who don't like bugs, are recommended to wait for a beta release.

Building
========

To build everything, just run "make" (requires GNU Make and GCC)

For more information, see BUILDING.txt .

Directory layout
===========

 3rdparty/         - External dependencies.

 build/            - Created during build process, contains all build artifacts.

 docs/             - Documentation.

 include/          - Public include files. This is the directory to add to your include dirs.

 notes/            - Miscellaneous notes and plan files.

 src/              - Source code.

 src/ca/           - Builtin Circa scripts.

 src/unit_tests/   - C++ based unit tests.

 tests/            - Script-based tests.

 tools/            - Tools used for development of Circa itself.

Documentation
=============

Refer to the /docs folder.

License
=======

Unless otherwise specified, the contents of this project are freely available to the
public under the MIT license. See the LICENSE file for more information.

Author
======

Andy Fischer - reachable on Github as 'andyfischer'
