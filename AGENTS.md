# AGENTS.md

## AI usage policy

AI-generated code is not merged without thorough human review and
supervision. No AI agent has unsupervised write access. Every AI-authored
change is audited for correctness, style, and safety before commit.

## Build and test

    mkdir build && cd build && cmake .. && make -j$(nproc)
    ./commandline_tests

## Testing constraints

`InteractiveBackend` cannot be unit tested directly. Its constructor saves
terminal attributes in a file-static global (shared across instances) and
spawns threads that block on stdin. Testing it requires either a pty
harness or extracting logic into a separate testable unit.

Tests that construct `Commandline` and depend on synchronous `on_write`
delivery must set `COMMANDLINE_FORCE_BUFFERED=1` in the environment before
construction. Without it, a tty stdin selects `InteractiveBackend`, whose
`on_write` fires asynchronously.

## Commit style

Lowercase imperative subject, blank line, bullet list body.
