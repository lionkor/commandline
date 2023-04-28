#pragma once

namespace impl {
bool is_interactive();
void init_terminal();
void reset_terminal();
int getchar_no_echo();
bool is_shift_pressed(bool forward);
}

#if defined(PLATFORM_WINDOWS) && PLATFORM_WINDOWS
#define WINDOWS
#elif defined(PLATFORM_LINUX) && PLATFORM_LINUX
#define UNIX
#endif
