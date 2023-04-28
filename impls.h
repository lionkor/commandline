#pragma once

namespace impl {
bool is_interactive();
void init_terminal();
void reset_terminal();
int getchar_no_echo();
bool is_shift_pressed(bool forward);
int get_terminal_width();
}

#if defined(PLATFORM_WINDOWS) && PLATFORM_WINDOWS
#define WINDOWS
#elif defined(PLATFORM_LINUX) && PLATFORM_LINUX
#define UNIX
#else
#error "define PLATFORM_LINUX=1 or PLATFORM_WINDOWS=1"
#endif
