#include "impls.h"

#if defined(PLATFORM_WINDOWS) && PLATFORM_WINDOWS
#include <array>
#include <conio.h>
#include <io.h>
#include <stdio.h>
#include <windows.h>

bool impl::is_interactive() {
    return _isatty(_fileno(stdout)) || _isatty(_fileno(stdin));
}

void impl::init_terminal() {
    SetConsoleOutputCP(CP_UTF8);
    {
        HANDLE hConsole_c = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        GetConsoleMode(hConsole_c, &dwMode);
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hConsole_c, dwMode);
    }
    {
        HANDLE hConsole_c = ::GetStdHandle(STD_INPUT_HANDLE);
        DWORD dwMode = 0;
        ::GetConsoleMode(hConsole_c, &dwMode);
        dwMode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
        dwMode |= ENABLE_VIRTUAL_TERMINAL_INPUT;
        ::SetConsoleMode(hConsole_c, dwMode);
    }
}

void impl::reset_terminal() {
}

int impl::getchar_no_echo() {
    return _getch();
}

bool impl::getchar_utf8_no_echo(char (&character)[7], size_t& used) {
    used = 0;
    HANDLE hConsole_c = GetStdHandle(STD_INPUT_HANDLE);

    wchar_t input[3];
    DWORD dummy = 0;
    DWORD lpNumberOfCharsRead = 1;

    if (!ReadConsoleW(hConsole_c, input, 1, &dummy, nullptr)) {
        return false;
    }

    if (IS_HIGH_SURROGATE(input[0])) {
        if (!ReadConsoleW(hConsole_c, input + 1, 1, &dummy, nullptr)) {
            return false;
        }
        lpNumberOfCharsRead += 1;
    }
    input[lpNumberOfCharsRead] = 0;
    used = WideCharToMultiByte(CP_UTF8, 0, input, lpNumberOfCharsRead, character, 7, nullptr, nullptr);
    if (!used) {
        return false;
    }
    return true;
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
