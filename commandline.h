#ifndef COMMANDLINE_H__
#define COMMANDLINE_H__

#include <atomic>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

class Commandline final {
private:
    std::thread m_io_thread;
    std::atomic<bool> m_shutdown { false };

    mutable std::mutex m_to_write_mutex;
    std::queue<std::string> m_to_write;
    mutable std::mutex m_to_read_mutex;
    std::queue<std::string> m_to_read;

    void io_thread_main();

public:
    Commandline();
    ~Commandline();

    bool has_command() const;
    void write(const std::string& str);
    std::string get_command();
};

#endif // COMMANDLINE_H
