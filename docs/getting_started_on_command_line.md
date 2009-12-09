
Getting Started (on the command line)
-------------------------------------

This document describes how to start using the 'circa' command-line binary. This tool is for running Circa scripts with console output. It's most useful if you want to learn or experiment with Circa syntax. The command-line tool doesn't provide any graphical abilities; for a graphical program use Plastic instead. 

### Downloading

To download, find the link for your platform at http://circa-lang.org . Unzip the package somewhere on your machine. This download just contains a single binary, with no install process.

### Running

Open up a command prompt and navigate to the unzipped folder. Now you can enter the command `circa` to execute the binary.

##### Running tests

Calling `circa` with no arguments will execute all its unit tests. You should see output like this:

    Ran 274 tests. All tests passed.

This command is probably only useful if you are developing on Circa.

##### Executing a file

Calling `circa <filename>` will tell it to run the given filename. Try creating a file called 'test.ca' using a text editor, with the following contents:

    print('Hello world')

Then run it with `circa test.ca`, and you should see the 'Hello world' message.

Now you can expand on that test file and play around with more complicated scripts. All of the scripts run by this tool should probably contain a few calls to print() since that's the easiest way to see the results of your program.

There are a few sample scripts available here (../samples), and this page (syntax.md) has details on the different kinds of syntax you can use.

##### REPL

Circa has a REPL (read-eval-print-loop), which is a quick way to experiment with syntax. To launch it, enter the command `circa -repl`. If successful you'll see a `> ` prompt. You can then enter any valid statement, and you'll see the result printed out.

Examples:

    > 1 + 1
    2
    > def hi()::number return 0.5 end
    def hi()::number return 0.5 end
    > sqrt(hi())
    0.707107

Note, the REPL currently doesn't support multiline input, if you want to write a block of code (like a function, for-loop, or if-block) then currently you'll have to put it on one line.

The REPL has a few meta-commands that you can see by typing `/help`. One of these is the `/raw` command, which displays raw compiled output:

    > /raw
    Displaying raw output
    > a = 1 + 2
    3
    $1920 value() :: int == 1
    $1925 value() :: int == 2
    $192a 'a' add_i($1920, $1925) :: int == 3

This is useful for debugging.

To exit the REPL, enter `/exit' or hit Control-C.

##### Other options

The command-line binary has a few other options, you can see them by entering the command `circa -help`.
