#include "BufferedBackend.h"
#include <iostream>

bool lk::BufferedBackend::has_command() const {
    std::lock_guard<std::mutex> lock(m_cmd_mtx);
    return !m_input_queue.empty();
}
void lk::BufferedBackend::write(const std::string& str) {
    std::lock_guard<std::mutex> lock(m_out_mtx);
    std::cout << str << std::endl;
    if (on_write) {
        on_write(str);
    }
}
std::string lk::BufferedBackend::get_command() {
    std::lock_guard<std::mutex> lock(m_cmd_mtx);
    auto cmd = std::move(m_input_queue.front());
    m_input_queue.pop_front();
    return cmd;
}
bool lk::BufferedBackend::history_enabled() const {
    return false;
}
void lk::BufferedBackend::enable_history() {
}
void lk::BufferedBackend::disable_history() {
}
void lk::BufferedBackend::set_history_limit(size_t) {
}
size_t lk::BufferedBackend::history_size() const {
    return 0;
}
void lk::BufferedBackend::clear_history() {
}
const std::vector<std::string>& lk::BufferedBackend::history() const {
    static const std::vector<std::string> empty_vec {};
    return empty_vec;
}
void lk::BufferedBackend::set_history(const std::vector<std::string>&) {
}

void lk::BufferedBackend::set_prompt(const std::string& p) {
    std::lock_guard<std::mutex> lock(m_prompt_mtx);
    m_prompt = p;
}

std::string lk::BufferedBackend::prompt() const {
    std::lock_guard<std::mutex> lock(m_prompt_mtx);
    return m_prompt;
}
void lk::BufferedBackend::enable_key_debug() {
}
void lk::BufferedBackend::disable_key_debug() {
}
lk::BufferedBackend::BufferedBackend(const std::string& prompt) {
    std::lock_guard<std::mutex> lock(m_prompt_mtx);
    m_prompt = prompt;
    m_thread = std::thread([this] { thread_main(); });
}
lk::BufferedBackend::~BufferedBackend() {
    {
        std::lock_guard<std::mutex> lock(m_shutdown_mtx);
        m_shutdown = true;
    }
    m_thread.join();
}
void lk::BufferedBackend::thread_main() {
    std::string str;
    while (std::getline(std::cin, str)) {
        {
            std::lock_guard<std::mutex> lock(m_shutdown_mtx);
            if (m_shutdown) {
                break;
            }
        }
        {
            std::lock_guard<std::mutex> lock(m_cmd_mtx);
            m_input_queue.push_back(str);
        }
        if (on_command) {
            on_command(*this);
        }
    }
}
