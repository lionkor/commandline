#include "commandline.h"
#include <functional>
#include <mutex>

#if !defined(_WIN32)
#include <stdio.h>
#include <termios.h>
#endif

Commandline::Commandline()
    : m_io_thread(std::bind(&Commandline::io_thread_main, this)) {
}

Commandline::~Commandline() {
    m_shutdown.store(true);
    m_io_thread.join();
}

static char getchar_no_echo();

#if defined(_WIN32)
static char getchar_no_echo() {
    return _getch();
}
#else

static struct termios old, current;

static void initTermios(int echo) {
    tcgetattr(0, &old);
    current = old;
    current.c_lflag &= ~ICANON;
    if (echo) {
        current.c_lflag |= ECHO;
    } else {
        current.c_lflag &= ~ECHO;
    }
    tcsetattr(0, TCSANOW, &current);
}

static void resetTermios() {
    tcsetattr(0, TCSANOW, &old);
}

static char getch_(int echo) {
    char ch;
    initTermios(echo);
    ch = getchar();
    resetTermios();
    return ch;
}

static char getchar_no_echo() {
    return getch_(false);
}
#endif // WIN32

void Commandline::io_thread_main() {
    std::mutex str_mutex;
    std::string str;
    int cursor_pos = 0;

    std::thread input_thread([&] {
        while (true) {
            char c = 0;
            while (c != '\n') {
                c = getchar_no_echo();
                std::lock_guard<std::mutex> guard(str_mutex);
                if (c == '\b' || c == 127) { // backspace or other delete sequence
                    if (!str.empty()) {
                        --cursor_pos;
                        str.pop_back();
                        printf("\x1b[2K\x1b[1000D%s", str.c_str());
                        fflush(stdout);
                    }
                } else if (isprint(c)) {
                    str += c;
                    ++cursor_pos;
                    putchar(c);
                }
            }
            std::lock_guard<std::mutex> guard(m_to_read_mutex);
            m_to_read.push(str);
            str.clear();
            cursor_pos = 0;
        }
    });
    // fire & forget
    input_thread.detach();
    while (!m_shutdown.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        std::lock_guard<std::mutex> guard(m_to_write_mutex);
        if (m_to_write.empty()) {
            continue;
        } else {
            auto to_write = m_to_write.front();
            m_to_write.pop();
            printf("\x1b[2K\x1b[1000D%s\n", to_write.c_str());
            std::lock_guard<std::mutex> guard2(str_mutex);
            printf("%s", str.c_str());
            fflush(stdout);
        }
    }
}

void Commandline::write(const std::string& str) {
    std::lock_guard<std::mutex> guard(m_to_write_mutex);
    m_to_write.push(str);
}

bool Commandline::has_command() const {
    std::lock_guard<std::mutex> guard(m_to_read_mutex);
    return !m_to_read.empty();
}

std::string Commandline::get_command() {
    std::lock_guard<std::mutex> guard(m_to_read_mutex);
    auto res = m_to_read.front();
    m_to_read.pop();
    return res;
}
