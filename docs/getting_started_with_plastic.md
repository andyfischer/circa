
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

Open up a text editor, and create a new file in this directory called 'test.ca' (or whatever you want to call it). Now launch Plastic using the command `plastic test.ca`. The currently-recommended way to edit Plastic documents is to have a text editor and Plastic open at the same time.

Now we'll get something on the screen. The first thing you'll want to do is fill the screen with a background, so that the previous frame is cleared. Add this line to your script:

    background(#006)

and hit save. The Plastic window should turn blue. You can use a different color here: the `#006` syntax is like CSS, it specifies the red, green, and blue parts of a color, using hexadecimal digits.

Next we'll get some text on the screen. Add the following line to display a line of text:

    draw_text(ui_font_medium, "Hello world", #fff, [200 200])

When you hit save, you should see "Hello world" displayed in white. Here is what each of that function's arguments means:

  * `ui_font_medium` - This is the font to use, ui_font_medium is loaded automatically inside runtime.ca. You can load a different font with `text.load_font`.
  * `"Hello world"` - The text to display
  * `#fff` - The text color, all white
  * `[200 200]` - The position on the screen. Coordinates are specified in pixels, where the top left of the screen is [0 0].

Once you have the draw_text function working, you can use that to experiment with some code. Change the `draw_text` line to be the following:

    s = concat('1 + 1 =' 1 + 1)
    draw_text(ui_font_medium, s, #fff, [200 200])

You can then change the arguments to the `concat()` function and see what the results of any expression would be.

### Handling errors

If your code causes an error, you'll see a message show up in the console window used to launch Plastic. (In a future version, errors will be displayed in the main Plastic window). You should be able to fix the offending code; Plastic will reattempt to run it whenever the file is saved.

### Work in progress

This document is still a work in progress
