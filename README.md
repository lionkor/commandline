# commandline
[![Linux Build](https://github.com/lionkor/commandline/workflows/CMake-Linux/badge.svg?branch=master)](https://github.com/lionkor/commandline/actions?query=workflow%3ACMake-Linux)
[![Windows Build](https://github.com/lionkor/commandline/workflows/CMake-Windows/badge.svg?branch=master)](https://github.com/lionkor/commandline/actions?query=workflow%3ACMake-Windows)
[![CodeFactor](https://www.codefactor.io/repository/github/lionkor/commandline/badge)](https://www.codefactor.io/repository/github/lionkor/commandline)

A cross-platfrom C++11 commandline for use in servers and terminal chat software. Provides very simple asynchronous input/output. 

Callback-based i/o supported, as well as history, custom prompts, and more (see [features](#features))

## Example

```cpp
#include "commandline.h"

int main() {
    Commandline com;
    while (true) {
        if (com.has_command()) {
            auto command = com.get_command();
            com.write("got command: " + command);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        com.write("this is a message written with com.write");
    }
}
```
**Result:**

![main.cpp demo gif](https://github.com/lionkor/commandline/blob/master/media/output.gif)

## Features

- Asynchronous I/O:
	The user can type into stdin while the program spams output onto stdout, with no visual issues. A command or message is committed with the Enter/Return key.

- Thread-safety:
	All output is buffered internally and protected with mutexes, so `write()` can be called by many threads at the same time without issues. Performance-wise this makes little impact, in our testing, as compared to usual printf() or std::cout logging (it's much faster than the latter in common scenarios).

- History:
	History of all commands entered is saved, if the history was enabled with `Commandline::enable_history()`. The history can be navigated like expected, with the up- and down-arrow keys, as well as cleared by the program, saved and restored, and more.

- Cursor movement:
	Even though an internal buffer is used (and the usual buffered input is disabled), the cursor can be moved as usual with the left- and right-arrow keys, and moved to the front and back with the HOME and END keys.

- `has_command()` and `get_command()`, or `on_command` callback:
	All these are supported as ways to get a line that was entered by the user, allowing many ways to integrate the library into existing applications.

- Output to file:
	Supports specifying a file to write all output into, as a logging mechanism!

- Cross-platform:
	Works on any POSIX system with a terminal that supports ANSI (all of the ones you can find, probably), as well as WinAPI console applications and Microsoft CMD.

## How to build

Run `cmake . `. 
Then:
- On Unix, this will generate a unix `Makefile`, which you can then run with `make`.
- On Windows, this will generate a VS project which you can open.

It should have then built the library, which you can link against.

You could also put `add_subdirectory(commandline)` to your CMakeLists, if you clone the repo in the same folder. 

NOTE: To build the example from above (included in `main.cpp`), set `BUILD_EXAMPLES` to `ON` in your CMake configuration.

## Why not X?

### Why not GNU getline?

Because it's GPL licensed, which makes it difficult to integrate into more permissive licensed or closed-source / commercial projects. `commandline` aims to be usable by anyone, as long as MIT license terms are followed.

### Why not support autocomplete?

`commandline` will get (optional) autocomplete support in the future, it's being worked on.

## How to use

1. Construct a `Commandline` instance. A prompt can optionally be passed in this constructor, or via `set_prompt`.

```cpp
Commandline com;
```

2. Query for new commands in a loop - this should usually be your main server loop or something similar. 

```cpp
bool is_running = true;
while (is_running) {
  // check if something was entered
  if (com.has_command()) {
    // grab the command from the queue
    // note: undefined if has_command wasn't checked beforehand
    std::string command = com.get_command();
    if (command == "quit") {
      is_running = false;
    }
  }
}
```

3. To write to the commandline, use `Commandline::write`.

```cpp
com.write("hello, world!");
```

## How does it work?

To support reading and writing at the same time, it's using VT100 ANSI escape codes. This means that, on windows, you need to [enable those for your CMD terminal](https://docs.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences?redirectedfrom=MSDN).

Before a new line is output into stdout, the current input line is cleared, the output line is written, and the input line is rewritten in the line below, positioning the cursor where it was, as if nothing happened. This results in the illusion that the input line is fixed below the output, which is a very natural design.

In most terminals and in Microsoft's CMD.exe (as well as standalone WinAPI console applications), this happens instantly and no visual glitches appear, which makes it very fun and smooth to use.

## Used In

Commandline is used in https://github.com/BeamMP/BeamMP-Server!
