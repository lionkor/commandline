#pragma once

#include "Backend.h"

#include <deque>
#include <mutex>
#include <thread>

namespace lk {

class BufferedBackend : public Backend {
public:
    explicit BufferedBackend(const std::string& prompt);
    ~BufferedBackend() override;

    bool has_command() const override;
    void write(const std::string& str) override;
    std::string get_command() override;
    bool history_enabled() const override;
    void enable_history() override;
    void disable_history() override;
    void set_history_limit(size_t count) override;
    size_t history_size() const override;
    void clear_history() override;
    const std::vector<std::string>& history() const override;
    void set_history(const std::vector<std::string>& history) override;
    void set_prompt(const std::string& p) override;
    std::string prompt() const override;
    void enable_key_debug() override;
    void disable_key_debug() override;

private:
    void thread_main();

    mutable std::mutex m_cmd_mtx {};
    mutable std::mutex m_out_mtx {};
    mutable std::mutex m_prompt_mtx {};
    mutable std::mutex m_shutdown_mtx {};
    bool m_shutdown = false;
    std::deque<std::string> m_input_queue {};
    std::string m_prompt;
    std::thread m_thread;
};

}
