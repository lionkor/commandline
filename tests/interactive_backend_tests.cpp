#include "doctest.h"
#include "backends/InteractiveBackend.h"

#include <vector>
#include <string>

namespace lk {

struct InteractiveBackendAccess {
    static void go_to_begin(InteractiveBackend& b) { b.go_to_begin(); }
    static void go_to_end(InteractiveBackend& b) { b.go_to_end(); }
    static void go_left(InteractiveBackend& b) { b.go_left(); }
    static void go_right(InteractiveBackend& b) { b.go_right(); }
    static void handle_backspace(InteractiveBackend& b) { b.handle_backspace(); }
    static void handle_delete(InteractiveBackend& b) { b.handle_delete(); }
    static void add_char(InteractiveBackend& b, char c) { b.add_to_current_buffer(c); }
    static void clear_suggestions(InteractiveBackend& b) { b.clear_suggestions(); }
    static bool cancel_autocomplete(InteractiveBackend& b) { return b.cancel_autocomplete_suggestion(); }

    static std::string buffer(const InteractiveBackend& b) { return b.m_current_buffer; }
    static int cursor(const InteractiveBackend& b) { return b.m_cursor_pos; }

    static void set_buffer(InteractiveBackend& b, const std::string& s) {
        b.m_current_buffer = s;
        b.m_cursor_pos = static_cast<int>(s.size());
    }

    static void set_suggestions(InteractiveBackend& b, const std::vector<std::string>& v) {
        b.m_autocomplete_suggestions = v;
        b.m_autocomplete_index = 0;
        b.m_buffer_before_autocomplete = b.m_current_buffer;
    }
};

} // namespace lk

using namespace lk;

TEST_CASE("go_to_begin") {
    InteractiveBackend b;
    InteractiveBackendAccess::set_buffer(b, "hello");
    InteractiveBackendAccess::go_left(b);
    InteractiveBackendAccess::go_left(b); // cursor at 3
    CHECK(InteractiveBackendAccess::cursor(b) == 3);
    InteractiveBackendAccess::go_to_begin(b);
    CHECK(InteractiveBackendAccess::cursor(b) == 0);
    CHECK(InteractiveBackendAccess::buffer(b) == "hello");
}

TEST_CASE("go_to_begin empty buffer") {
    InteractiveBackend b;
    InteractiveBackendAccess::go_to_begin(b);
    CHECK(InteractiveBackendAccess::cursor(b) == 0);
}

TEST_CASE("go_to_end") {
    InteractiveBackend b;
    InteractiveBackendAccess::set_buffer(b, "hello");
    InteractiveBackendAccess::go_to_begin(b);
    CHECK(InteractiveBackendAccess::cursor(b) == 0);
    InteractiveBackendAccess::go_to_end(b);
    CHECK(InteractiveBackendAccess::cursor(b) == 5);
}

TEST_CASE("go_to_end empty buffer") {
    InteractiveBackend b;
    InteractiveBackendAccess::go_to_end(b);
    CHECK(InteractiveBackendAccess::cursor(b) == 0);
}

TEST_CASE("go_left") {
    InteractiveBackend b;
    InteractiveBackendAccess::set_buffer(b, "ab");
    CHECK(InteractiveBackendAccess::cursor(b) == 2);
    InteractiveBackendAccess::go_left(b);
    CHECK(InteractiveBackendAccess::cursor(b) == 1);
    InteractiveBackendAccess::go_left(b);
    CHECK(InteractiveBackendAccess::cursor(b) == 0);
    InteractiveBackendAccess::go_left(b);
    CHECK(InteractiveBackendAccess::cursor(b) == 0); // clamped
}

TEST_CASE("go_right") {
    InteractiveBackend b;
    InteractiveBackendAccess::set_buffer(b, "ab");
    InteractiveBackendAccess::go_to_begin(b);
    CHECK(InteractiveBackendAccess::cursor(b) == 0);
    InteractiveBackendAccess::go_right(b);
    CHECK(InteractiveBackendAccess::cursor(b) == 1);
    InteractiveBackendAccess::go_right(b);
    CHECK(InteractiveBackendAccess::cursor(b) == 2);
    InteractiveBackendAccess::go_right(b);
    CHECK(InteractiveBackendAccess::cursor(b) == 2); // clamped
}

TEST_CASE("handle_backspace middle") {
    InteractiveBackend b;
    InteractiveBackendAccess::set_buffer(b, "abc");
    InteractiveBackendAccess::go_left(b); // cursor at 2
    InteractiveBackendAccess::handle_backspace(b);
    CHECK(InteractiveBackendAccess::buffer(b) == "ac");
    CHECK(InteractiveBackendAccess::cursor(b) == 1);
}

TEST_CASE("handle_backspace at start") {
    InteractiveBackend b;
    InteractiveBackendAccess::set_buffer(b, "abc");
    InteractiveBackendAccess::go_to_begin(b); // cursor at 0
    InteractiveBackendAccess::handle_backspace(b);
    // cursor was 0, clamped to 0, erases char at position 0
    CHECK(InteractiveBackendAccess::buffer(b) == "bc");
    CHECK(InteractiveBackendAccess::cursor(b) == 0);
}

TEST_CASE("handle_backspace empty buffer") {
    InteractiveBackend b;
    InteractiveBackendAccess::handle_backspace(b);
    CHECK(InteractiveBackendAccess::buffer(b) == "");
    CHECK(InteractiveBackendAccess::cursor(b) == 0);
}

TEST_CASE("handle_delete middle") {
    InteractiveBackend b;
    InteractiveBackendAccess::set_buffer(b, "abc");
    InteractiveBackendAccess::go_to_begin(b);
    InteractiveBackendAccess::go_right(b); // cursor at 1 (on 'b')
    InteractiveBackendAccess::handle_delete(b);
    CHECK(InteractiveBackendAccess::buffer(b) == "ac");
    CHECK(InteractiveBackendAccess::cursor(b) == 1);
}

TEST_CASE("handle_delete at end") {
    InteractiveBackend b;
    InteractiveBackendAccess::set_buffer(b, "abc");
    InteractiveBackendAccess::handle_delete(b); // cursor at end, nothing to delete
    CHECK(InteractiveBackendAccess::buffer(b) == "abc");
    CHECK(InteractiveBackendAccess::cursor(b) == 3);
}

TEST_CASE("handle_delete empty buffer") {
    InteractiveBackend b;
    InteractiveBackendAccess::handle_delete(b);
    CHECK(InteractiveBackendAccess::buffer(b) == "");
}

TEST_CASE("add_to_current_buffer empty") {
    InteractiveBackend b;
    InteractiveBackendAccess::add_char(b, 'x');
    CHECK(InteractiveBackendAccess::buffer(b) == "x");
    CHECK(InteractiveBackendAccess::cursor(b) == 1);
}

TEST_CASE("add_to_current_buffer middle") {
    InteractiveBackend b;
    InteractiveBackendAccess::set_buffer(b, "ac");
    InteractiveBackendAccess::go_to_begin(b);
    InteractiveBackendAccess::go_right(b); // cursor at 1
    InteractiveBackendAccess::add_char(b, 'b');
    CHECK(InteractiveBackendAccess::buffer(b) == "abc");
    CHECK(InteractiveBackendAccess::cursor(b) == 2);
}

TEST_CASE("add_to_current_buffer end") {
    InteractiveBackend b;
    InteractiveBackendAccess::set_buffer(b, "ab");
    InteractiveBackendAccess::add_char(b, 'c');
    CHECK(InteractiveBackendAccess::buffer(b) == "abc");
    CHECK(InteractiveBackendAccess::cursor(b) == 3);
}

TEST_CASE("cancel_autocomplete inactive") {
    InteractiveBackend b;
    CHECK(!InteractiveBackendAccess::cancel_autocomplete(b));
}

TEST_CASE("cancel_autocomplete active") {
    InteractiveBackend b;
    InteractiveBackendAccess::set_buffer(b, "orig");
    InteractiveBackendAccess::set_suggestions(b, {"replacement"});
    InteractiveBackendAccess::set_buffer(b, "replacement"); // simulate applying suggestion
    bool cancelled = InteractiveBackendAccess::cancel_autocomplete(b);
    CHECK(cancelled);
    CHECK(InteractiveBackendAccess::buffer(b) == "orig");
    CHECK(InteractiveBackendAccess::cursor(b) == 4);
}

TEST_CASE("handle_backspace cancels autocomplete first") {
    InteractiveBackend b;
    InteractiveBackendAccess::set_buffer(b, "orig");
    InteractiveBackendAccess::set_suggestions(b, {"replacement"});
    InteractiveBackendAccess::set_buffer(b, "replacement");
    InteractiveBackendAccess::handle_backspace(b);
    // backspace should have cancelled autocomplete, restoring "orig"
    CHECK(InteractiveBackendAccess::buffer(b) == "orig");
}
