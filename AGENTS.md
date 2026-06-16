# AGENTS.md

## AI usage policy

This codebase is human-maintained. AI-generated code is not merged without
thorough human review and supervision. No AI agent has unsupervised write
access. Every AI-authored change is audited for correctness, style, and
safety before commit.

## Project overview

A C++11 terminal input library. Provides line editing with history,
autocomplete, and ANSI-aware output.

### Directory layout

```
src/
  commandline.h/.cpp       public API (class Commandline)
  backends/
    Backend.h              abstract interface
    InteractiveBackend.*   tty-backed, full line editing
    BufferedBackend.*      line-based, no terminal I/O
  helper/
    ansi.h/.cpp            ANSI escape code stripping
  impls.h                  platform function declarations
  linux_impl.cpp           POSIX termios implementation
  windows_impl.cpp         Windows console implementation
  main.cpp                 example / smoke-test program
tests/
  tests.cpp                unit tests (doctest)
  doctest.h                doctest single-header
```

### Build

```
mkdir build && cd build
cmake ..
make -j$(nproc)
./commandline_tests          # run tests
```

### Key architecture

- `Commandline` is the public API. It owns a `unique_ptr<Backend>`.
- `Backend` is the abstract interface: `has_command`, `get_command`, `write`,
  `on_command`, `on_write`, `on_autocomplete`, history, prompt, key_debug.
- `InteractiveBackend` spawns two threads:
  - I/O thread: drains the write queue, prints output with ANSI cursor
    management, calls `on_write`.
  - Input thread (detached): blocks on `getchar_no_echo()`, dispatches
    keystrokes, escape sequences, tab, backspace.
- `BufferedBackend` reads lines from `std::cin` synchronously. Used when
  stdin is not a tty, or when `COMMANDLINE_FORCE_BUFFERED=1` is set.
- Platform code lives in `impls.h` + `linux_impl.cpp` / `windows_impl.cpp`,
  selected via `PLATFORM_LINUX=1` or `PLATFORM_WINDOWS=1` compile definitions.

### Escape sequence handling

`InteractiveBackend::handle_escape_sequence()` dispatches ANSI escape
sequences. On UNIX, the pattern is:

1. `ESC` (0x1b) is caught in `input_thread_main`, which calls
   `handle_escape_sequence()`.
2. `handle_escape_sequence` reads `c2 = getchar_no_echo()`.
3. If `c2 == '['`, it reads `c3 = getchar_no_echo()` and dispatches.
4. Multi-byte sequences (`ESC[1~`, `ESC[4~`, `ESC[3~`) read an additional
   `c4` terminator.

On Windows, extended keys are prefixed with `0xe0` instead of `0x1b`.

Supported sequences:
| Key        | UNIX                | Windows      |
|------------|---------------------|--------------|
| UP         | `ESC[A`             | `0xe0 H`     |
| DOWN       | `ESC[B`             | `0xe0 P`     |
| LEFT       | `ESC[D`             | `0xe0 K`     |
| RIGHT      | `ESC[C`             | `0xe0 M`     |
| HOME       | `ESC[H`, `ESC[1~`   | `0xe0 0x47`  |
| END        | `ESC[F`, `ESC[4~`   | `0xe0 0x4f`  |
| DEL        | `ESC[3~`            | `0xe0 0x53`  |
| SHIFT+TAB  | `ESC[Z`             | —            |

Up/down navigation is gated on `history_enabled()`. All other keys
(left, right, home, end, del, shift+tab) work regardless.

### Testing

- Test framework: doctest (single-header in `tests/doctest.h`).
- Library compiled with `DOCTEST_CONFIG_DISABLE=1` to suppress test
  registration in production builds.
- Test target: `commandline_tests`, built from `tests/tests.cpp`.
- `InteractiveBackend` **cannot be unit tested directly**. Its constructor
  calls `impl::init_terminal()` (saves terminal attrs in a file-static
  global shared across instances) and spawns threads that block on stdin.
  Testing it requires a pty harness or extracting testable logic into a
  separate unit.
- Tests that construct `Commandline` and depend on synchronous `on_write`
  delivery must use `setenv("COMMANDLINE_FORCE_BUFFERED", "1", 1)` before
  construction to guarantee `BufferedBackend` regardless of tty state.

### Commit style

Lowercase imperative, single line subject, blank line, bullet list body.
Example:

    fix home/end key handling in interactive backend

    - ungate c2 == '[' from history_enabled()
    - fix ESC[1~ to consume trailing ~
    - add ESC[4~ end sequence

### Coding conventions

- C++11, no exceptions, no RTTI.
- Namespace `lk` for library types, `impl` for platform internals.
- Member variables prefixed with `m_`.
- Mutexes lock minimal scope; `std::lock_guard` everywhere.
- Backend callbacks (`on_command`, `on_write`, `on_autocomplete`) are wired
  through `Commandline::wire_callbacks()` — both constructors call it.
- Preprocessor guards `#if defined(UNIX)` / `#elif defined(WINDOWS)` for
  platform-specific code paths.
