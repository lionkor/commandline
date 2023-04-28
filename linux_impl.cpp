#include "impls.h"

#if defined(PLATFORM_LINUX) && PLATFORM_LINUX
#include <cstdio>
#include <pthread.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>

static struct termios s_original_termios;

bool impl::is_interactive() {
    return isatty(STDIN_FILENO) && isatty(STDOUT_FILENO);
}

void impl::init_terminal() {
    tcgetattr(0, &s_original_termios);
}

void impl::reset_terminal() {
    tcsetattr(0, TCSANOW, &s_original_termios);
    std::puts("\n"); // obligatory
}

namespace detail {
static struct termios s_old_termios, s_current_termios;

static void init_termios(bool echo) {
    tcgetattr(0, &s_old_termios);
    s_current_termios = s_old_termios;
    s_current_termios.c_lflag &= ~ICANON;
    if (echo) {
        s_current_termios.c_lflag |= ECHO;
    } else {
        s_current_termios.c_lflag &= ~ECHO;
    }
    tcsetattr(0, TCSANOW, &s_current_termios);
}

static void reset_termios() {
    tcsetattr(0, TCSANOW, &s_old_termios);
}

static int getch_(bool echo) {
    int ch;
    init_termios(echo);
    ch = getchar();
    reset_termios();
    return ch;
}
} // namespace detail

int impl::getchar_no_echo() {
    return detail::getch_(false);
}

bool impl::is_shift_pressed(bool forward) {
    return forward;
}

int impl::get_terminal_width() {
    struct winsize w;
    int ret = ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    if (ret == -1) {
        return 80; // some sane default
    } else {
        return w.ws_col;
    }
}

#endif
