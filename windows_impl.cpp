#include "impls.h"

#if defined(PLATFORM_WINDOWS) && PLATFORM_WINDOWS
#include <array>
#include <conio.h>
#include <windows.h>

bool impl::is_interactive() {
    return true;
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

#endif
