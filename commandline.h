#ifndef COMMANDLINE_H__
#define COMMANDLINE_H__

#ifdef WIN32
// for windows, `max` may be defined which breaks
// std::numeric_limits<T>::max()
#undef max
#endif

#include <atomic>
#include <filesystem>
#include <fstream>
#include <functional>
#include <limits>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

class Commandline final {
public:
    explicit Commandline(const std::string& prompt = "");
    Commandline(const Commandline&) = delete;
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
    void set_prompt(const std::string& p);
    std::string prompt() const;
    bool write_to_file_enabled() const { return m_write_to_file; }
    [[nodiscard]] bool enable_write_to_file(const std::filesystem::path& path);
    void disable_write_to_file() { m_write_to_file = false; }

    // gets called when a command is ready
    std::function<void(Commandline&)> on_command { nullptr };

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

    std::string m_prompt;

    std::thread m_io_thread;
    std::atomic<bool> m_shutdown { false };
    bool m_interactive { true };

    mutable std::mutex m_to_write_mutex;
    std::queue<std::string> m_to_write;
    mutable std::mutex m_to_read_mutex;
    std::queue<std::string> m_to_read;
    bool m_history_enabled { false };
    bool m_write_to_file { false };
    std::ofstream m_logfile;
    std::filesystem::path m_logfile_path {};
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
