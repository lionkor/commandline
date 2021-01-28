#ifndef COMMANDLINE_H__
#define COMMANDLINE_H__

#include <atomic>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

class Commandline final {
public:
    Commandline();
    ~Commandline();

    bool has_command() const;
    void write(const std::string& str);
    std::string get_command();
    bool history_enabled() const { return m_history_enabled; }
    void enable_history() { m_history_enabled = true; }
    void disable_history() { m_history_enabled = false; }
    void set_history_limit(size_t count);
    size_t history_size() const;
    void clear_history();

private:
    void io_thread_main();
    void input_thread_main();

    void add_to_history(const std::string& str);
    void go_back_in_history();
    void go_forward_in_history();
    void add_to_current_buffer(char c);
    void update_current_buffer_view();
    void handle_escape_sequence();
    void handle_backspace();

    std::thread m_io_thread;
    std::atomic<bool> m_shutdown { false };

    mutable std::mutex m_to_write_mutex;
    std::queue<std::string> m_to_write;
    mutable std::mutex m_to_read_mutex;
    std::queue<std::string> m_to_read;
    bool m_history_enabled { false };
    mutable std::mutex m_history_mutex;
    std::vector<std::string> m_history;
    std::string m_history_temp_buffer;
    size_t m_history_index { 0 };
    size_t m_history_limit = std::numeric_limits<size_t>::max() - 1;
    std::mutex m_current_buffer_mutex;
    std::string m_current_buffer;
    int m_cursor_pos = 0;
};

#endif // COMMANDLINE_H
