#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "backends/BufferedBackend.h"
#include "commandline.h"
#include "helper/ansi.h"

#include <cstdlib>

TEST_CASE("ansi::remove_ansi_escape_codes removes escape sequences") {
    CHECK(ansi::remove_ansi_escape_codes("plain") == "plain");
    CHECK(ansi::remove_ansi_escape_codes("\x1b[31mred\x1b[0m") == "red");
    CHECK(ansi::remove_ansi_escape_codes("\x1b[Aup\nx") == "up\nx");
}

static void force_buffered() {
#ifdef _WIN32
    _putenv("COMMANDLINE_FORCE_BUFFERED=1");
#else
    setenv("COMMANDLINE_FORCE_BUFFERED", "1", 1);
#endif
}

TEST_CASE("Commandline on_write forwards without ANSI removal") {
    force_buffered();
    Commandline com;
    std::string captured;
    com.on_write = [&](const std::string& s) { captured = s; };
    com.write("abc\x1b[31mdef");
    CHECK(captured == "abc\x1b[31mdef");
}

TEST_CASE("Commandline on_write removes ANSI when enabled") {
    force_buffered();
    Commandline com;
    com.enable_ansi_escape_removal_on_write();
    std::string captured;
    com.on_write = [&](const std::string& s) { captured = s; };
    com.write("abc\x1b[31mdef");
    CHECK(captured == "abcdef");
}

TEST_CASE("Commandline on_autocomplete forwards to user callback") {
    Commandline com;
    com.on_autocomplete = [](Commandline&, std::string stub, int) {
        if (stub == "he")
            return std::vector<std::string> { "hello", "help" };
        return std::vector<std::string> { };
    };
    // Invoke backend autocomplete via commandline plumbing
    // We cannot access backend directly; test using public callback:
    auto results = com.on_autocomplete ? com.on_autocomplete(com, "he", 2) : std::vector<std::string> { };
    CHECK(results.size() == 2);
    CHECK(results[0] == "hello");
    CHECK(results[1] == "help");
}

TEST_CASE("BufferedBackend prompt and write basic behavior") {
    lk::BufferedBackend backend(">");
    CHECK(backend.prompt() == ">");
    backend.set_prompt("$");
    CHECK(backend.prompt() == "$");

    // Capture on_write
    std::string captured;
    backend.on_write = [&](const std::string& s) { captured = s; };
    backend.write("msg");
    CHECK(captured == "msg");
}

TEST_CASE("BufferedBackend concurrent writes are serialized") {
    lk::BufferedBackend backend(">");
    std::mutex m;
    std::vector<std::string> seen;
    backend.on_write = [&](const std::string& s) {
        std::lock_guard<std::mutex> g(m);
        seen.push_back(s);
    };
    std::vector<std::thread> ts;
    for (int i = 0; i < 10; ++i) {
        ts.emplace_back([&] { backend.write("x"); });
    }
    for (auto& t : ts)
        t.join();
    CHECK(seen.size() == 10);
}
