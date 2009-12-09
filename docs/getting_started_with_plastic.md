
Getting Started with Plastic
----------------------------

This document describes how to start using the Plastic tool. Plastic executes Circa scripts in a graphical environment, and provides a bunch of built-in functions which are not available in the Circa command-line tool.

### Downloading

Find the link for your platform at http://circa-lang.org. Unzip the package somewhere on your machine. This package should contain:

 - 'plastic' or 'plastic.exe' - the main executable
 - an 'assets' folder, which contains some built-in fonts
 - a 'bin' folder, which contains some shared libraries
 - a 'demos' folder, which contains some games written in Plastic
 - the 'runtime.ca' file, which is a Circa file that declares all the Plastic-specific functions

All of these files (except for the 'demos' folder) must be present in order for Plastic to run.

### Running

Currently, the primary way of launching Plastic is to use the command-line. You can also launch Plastic by double-clicking the executable, but this will only launch the default game (as defined by `default_script_filename` in runtime.ca), and there's no easy way to make it open a different file. A future version of Plastic will have a file browser, so that you don't need to use the command line.

Anyway, open up a command prompt and navigate to the unzipped folder. Try running the command `plastic demos/pong.ca` - this should launch a Pong clone. There are a bunch of files inside the /demos folder that you can try (but beware as some of them don't work so well).

While running Plastic, there are a few keyboard shortcuts that have special actions, visit this page (plastic_key_shortcuts.md) to see a list.

### Creating

Open up a text editor, and create a new file in this directory called 'test.ca' (or whatever you want to call it). Now launch Plastic using the command `plastic test.ca`.

Work in progress..
