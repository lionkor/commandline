#include "ansi.h"
#include <string>

enum class State {
    Normal,
    Escaped,
};

std::string ansi::remove_ansi_escape_codes(const std::string& original) {
    std::string new_str {};
    new_str.reserve(original.size());

    State state = State::Normal;

    for (size_t i = 0; i < original.size(); ++i) {
        if (state == State::Normal && original[i] == '\x1b') {
            state = State::Escaped;
            continue;
        }

        if (state == State::Escaped) {
            switch (original[i]) {
            case 'A': // Up
            case 'B': // Down
            case 'C': // Right
            case 'D': // Left
            case 'E': // Next line
            case 'F': // Previous line
            case 'G': // Set column
            case 'H': // Set cursor position
            case 'J': // Erase display
            case 'K': // Erase line
            case 'T': // Scroll down
            case 'f': // Cursor position
            case 'm': // Graphics mode
                state = State::Normal;
                continue;
            }
        }

        if (state == State::Normal) {
            new_str.push_back(original[i]);
        }
    }

    return new_str;
}

#include "doctest.h"

TEST_CASE("ansi removal") {
    CHECK(ansi::remove_ansi_escape_codes("hello") == "hello");
    // Colors
    CHECK(ansi::remove_ansi_escape_codes("\x1b[1;2mhello world") == "hello world");
    // Cursor up
    CHECK(ansi::remove_ansi_escape_codes("\x1b[Ahello\nworld") == "hello\nworld");
    // Cursor down
    CHECK(ansi::remove_ansi_escape_codes("\x1b[Bfoo bar") == "foo bar");
    // Cursor right
    CHECK(ansi::remove_ansi_escape_codes("\x1b[Cfoo\nbar") == "foo\nbar");
    // Cursor left
    CHECK(ansi::remove_ansi_escape_codes("\x1b[Dfoo\tbar") == "foo\tbar");
    // Next line
    CHECK(ansi::remove_ansi_escape_codes("\x1b[Efoo\rbar") == "foo\rbar");
    // Previous line
    CHECK(ansi::remove_ansi_escape_codes("\x1b[Fhello world!") == "hello world!");
    // Set column
    CHECK(ansi::remove_ansi_escape_codes("\x1b[Gfoo bar baz") == "foo bar baz");
    // Set cursor position
    CHECK(ansi::remove_ansi_escape_codes("\x1b[Hfoo\nbar\nbaz") == "foo\nbar\nbaz");
    // Erase display
    CHECK(ansi::remove_ansi_escape_codes("\x1b[Jfoo\tbar\tbaz") == "foo\tbar\tbaz");
    // Erase line
    CHECK(ansi::remove_ansi_escape_codes("\x1b[Kfoo\rbar\rbaz") == "foo\rbar\rbaz");
    // Scroll down
    CHECK(ansi::remove_ansi_escape_codes("\x1b[Thello\nworld!") == "hello\nworld!");
    // Cursor position (alternative)
    CHECK(ansi::remove_ansi_escape_codes("\x1b[fhello\tworld!") == "hello\tworld!");
    // Graphics mode
    CHECK(ansi::remove_ansi_escape_codes("\x1b[mfoo bar baz") == "foo bar baz");
}
