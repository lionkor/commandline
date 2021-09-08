# commandline
[![Linux Build](https://github.com/lionkor/commandline/workflows/CMake-Linux/badge.svg?branch=master)](https://github.com/lionkor/commandline/actions?query=workflow%3ACMake-Linux)
[![Windows Build](https://github.com/lionkor/commandline/workflows/CMake-Windows/badge.svg?branch=master)](https://github.com/lionkor/commandline/actions?query=workflow%3ACMake-Windows)

[![CodeFactor](https://www.codefactor.io/repository/github/lionkor/commandline/badge)](https://www.codefactor.io/repository/github/lionkor/commandline)

A C++ commandline for use in servers and terminal chat software. Provides very simple asynchronous input/output.

Supports reading and writing at the same time, using VT100 ANSI escape codes. This means that, on windows, you need to [enable those for your CMD terminal](https://docs.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences?redirectedfrom=MSDN).

Has a history implementation, so you can enable history with `enable_history()`, and then navigate it with Esc\[A (Up-Arrow) and Esc\[B (Down-Arrow). You can set the history size with `set_history_size`, by default this is `std::numeric_limits<size_t>::max()`.

A prompt can be passed in the `Commandline::Commandline` constructor, or via `set_prompt`.

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

## How to build

Run `cmake . `. 
Then:
- On Unix, this will generate a unix `Makefile`, which you can then run with `make`.
- On Windows, this will generate a VS project which you can open.

It should have then built the library, which you can link against.

You could also put `add_subdirectory(commandline)` to your CMakeLists, if you clone the repo in the same folder. 

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

## Used In

Commandline is used in https://github.com/BeamMP/BeamMP-Server!
