#pragma once

#include "Backend.h"
#include <atomic>
#include <condition_variable>
#include <functional>
#include <limits>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace lk {

class InteractiveBackend : public Backend {
public:
    explicit InteractiveBackend(const std::string& prompt = "");
    InteractiveBackend(const InteractiveBackend&) = delete;
    ~InteractiveBackend() override;

    bool has_command() const override;
    void write(const std::string& str) override;
    std::string get_command() override;
    bool history_enabled() const override { return m_history_enabled; }
    void enable_history() override { m_history_enabled = true; }
    void disable_history() override { m_history_enabled = false; }
    void set_history_limit(size_t count) override;
    size_t history_size() const override;
    void clear_history() override;
    const std::vector<std::string>& history() const override { return m_history; }
    void set_history(const std::vector<std::string>& history) override {
        m_history = history;
        m_history_index = 0;
    }
    void set_prompt(const std::string& p) override;
    std::string prompt() const override;

    // key_debug writes escape-sequenced keys to stderr
    void enable_key_debug() override;
    void disable_key_debug() override;

private:
    void io_thread_main();
    void input_thread_main();

    void add_to_history(const std::string& str);
    void go_back_in_history();
    void go_forward_in_history();
    void add_to_current_buffer(char c);
    void update_current_buffer_view();
    void handle_escape_sequence(std::unique_lock<std::mutex>& guard);
    void handle_backspace();
    void handle_delete();
    void handle_tab(std::unique_lock<std::mutex>& guard, bool forward);
    void clear_suggestions();
    bool cancel_autocomplete_suggestion();
    void go_back();
    void go_forward();
    void go_right();
    void go_left();
    void go_to_begin();
    void go_to_end();

    size_t current_view_size();
    size_t current_view_offset();
    std::string current_view();
    size_t current_view_cursor_pos();

    std::string m_prompt;

    std::thread m_io_thread;
    std::atomic<bool> m_shutdown { false };
    bool m_key_debug { false };

    mutable std::mutex m_to_write_mutex;
    std::queue<std::string> m_to_write;
    std::condition_variable m_to_write_cond;
    mutable std::mutex m_to_read_mutex;
    std::queue<std::string> m_to_read;
    bool m_history_enabled { false };
    mutable std::mutex m_history_mutex;
    std::vector<std::string> m_history;
    std::string m_history_temp_buffer;
    size_t m_history_index { 0 };
    size_t m_history_limit = (std::numeric_limits<size_t>::max)() - 1;
    std::mutex m_current_buffer_mutex;
    std::string m_current_buffer;
    int m_cursor_pos = 0;
    std::vector<std::string> m_autocomplete_suggestions;
    size_t m_autocomplete_index = 0;
    std::string m_buffer_before_autocomplete;
};

}
