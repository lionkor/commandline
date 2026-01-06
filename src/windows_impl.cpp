#include "impls.h"

#if defined(PLATFORM_WINDOWS) && PLATFORM_WINDOWS
#include <array>
#include <conio.h>
#include <io.h>
#include <stdio.h>
#include <windows.h>

bool impl::is_interactive() {
    char* force_buffered = nullptr;
    size_t len = 0;
    if (_dupenv_s(&force_buffered, &len, "COMMANDLINE_FORCE_BUFFERED") == 0 && force_buffered != nullptr) {
        bool result = (*force_buffered == '1') ? false : true;
        free(force_buffered);
        if (!result) {
            return false;
        }
    }

    char* force_interactive = nullptr;
    len = 0;
    if (_dupenv_s(&force_interactive, &len, "COMMANDLINE_FORCE_INTERACTIVE") == 0 && force_interactive != nullptr) {
        bool result = (*force_interactive == '1') ? true : false;
        free(force_interactive);
        if (result) {
            return true;
        }
    }

    return _isatty(_fileno(stdout)) || _isatty(_fileno(stdin));
}

void impl::init_terminal() {
    HANDLE hConsole_c = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hConsole_c, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hConsole_c, dwMode);
}

void impl::reset_terminal() {
}

int impl::getchar_no_echo() {
    return _getch();
}

bool impl::is_shift_pressed(bool forward) {
    auto x = uint16_t(GetKeyState(VK_SHIFT));
    if (x > 1) {
        return false;
    }
    return forward;
}

int impl::get_terminal_width() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int ret = GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    if (ret) {
        return csbi.dwSize.X;
    } else {
        return 80; // some sane default
    }
}

#endif
