#pragma once

#include "backends/Backend.h"
#include <memory>

class Commandline final {
public:
    explicit Commandline(const std::string& prompt = "");

    bool has_command() const { return m_backend->has_command(); }
    void write(const std::string& str) { m_backend->write(str); }
    std::string get_command() { return m_backend->get_command(); }
    bool history_enabled() const { return m_backend->history_enabled(); }
    void enable_history() { m_backend->enable_history(); }
    void disable_history() { m_backend->disable_history(); }
    void set_history_limit(size_t count) { m_backend->set_history_limit(count); }
    size_t history_size() const { return m_backend->history_size(); }
    void clear_history() { m_backend->clear_history(); }
    const std::vector<std::string>& history() const { return m_backend->history(); }
    void set_history(const std::vector<std::string>& history) { m_backend->set_history(history); }
    void set_prompt(const std::string& p) { m_backend->set_prompt(p); }
    std::string prompt() const { return m_backend->prompt(); }

    // key_debug writes escape-sequenced keys to stderr
    void enable_key_debug() { m_backend->enable_key_debug(); }
    void disable_key_debug() { m_backend->disable_key_debug(); }

    // gets called when a command is ready
    std::function<void(Commandline&)> on_command { nullptr };

    // gets called when tab is pressed and new suggestions are requested
    std::function<std::vector<std::string>(Commandline&, std::string, int)> on_autocomplete { nullptr };

    // gets called on write(), for writing to a file or similar secondary logging system
    std::function<void(const std::string&)> on_write { nullptr };

private:
    std::unique_ptr<lk::Backend> m_backend;
};