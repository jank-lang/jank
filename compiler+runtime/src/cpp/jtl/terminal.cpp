#if defined(_WIN32)
  #define WIN32_LEAN_AND_MEAN

  #ifndef NOMINMAX
    #define NOMINMAX
  #endif

  #include <windows.h>
#else
  #include <sys/ioctl.h>
  #include <unistd.h>
#endif

#include <jtl/terminal.hpp>

namespace jtl::terminal
{
  static size default_size()
  {
    return { 80, 24 };
  }

  size get_size()
  {
#if defined(_WIN32)
    CONSOLE_SCREEN_BUFFER_INFO csbi{};

    if(GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
    {
      return { csbi.srWindow.Right - csbi.srWindow.Left + 1,
               csbi.srWindow.Bottom - csbi.srWindow.Top + 1 };
    }

    return default_size();
#else
    winsize w{};
    /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) */
    int const status{ ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) };
    if(w.ws_col == 0 || w.ws_row == 0 || status < 0)
    {
      return default_size();
    }
    return { w.ws_col, w.ws_row };
#endif
  }
}
