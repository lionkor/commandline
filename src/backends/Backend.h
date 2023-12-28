#pragma once

#include <functional>
#include <string>
#include <vector>

namespace lk {

class Backend {
public:
    Backend() = default;
    Backend(const Backend&) = delete;
    virtual ~Backend() = default;

    virtual bool has_command() const = 0;
    virtual void write(const std::string& str) = 0;
    virtual std::string get_command() = 0;
    virtual bool history_enabled() const = 0;
    virtual void enable_history() = 0;
    virtual void disable_history() = 0;
    virtual void set_history_limit(size_t count) = 0;
    virtual size_t history_size() const = 0;
    virtual void clear_history() = 0;
    virtual const std::vector<std::string>& history() const = 0;
    virtual void set_history(const std::vector<std::string>& history) = 0;
    virtual void set_prompt(const std::string& p) = 0;
    virtual std::string prompt() const = 0;

    // key_debug writes escape-sequenced keys to stderr
    virtual void enable_key_debug() = 0;
    virtual void disable_key_debug() = 0;

    // gets called when a command is ready
    std::function<void(Backend&)> on_command { nullptr };

    // gets called when tab is pressed and new suggestions are requested
    std::function<std::vector<std::string>(Backend&, std::string, int)> on_autocomplete { nullptr };

    // gets called on write(), for writing to a file or similar secondary logging system
    std::function<void(const std::string&)> on_write { nullptr };
};

}
