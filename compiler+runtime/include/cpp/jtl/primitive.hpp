#pragma once

#ifndef NDEBUG
  #define JANK_ASSERTIONS_ENABLED
#endif

namespace jtl
{
  using i8 = signed char;
  using u8 = unsigned char;

  using i16 = short;
  using u16 = unsigned short;

  using i32 = int;
  using u32 = unsigned int;

  using i64 = long long;
  using u64 = unsigned long long;

  using f32 = float;
  using f64 = double;

  using uptr = unsigned long long;
  using usize = uptr;
  using uhash = u32;
  using nullptr_t = decltype(nullptr);

  static_assert(sizeof(char) == 1);
  static_assert(sizeof(bool) == 1);
  static_assert(sizeof(i8) == 1);
  static_assert(sizeof(i16) == 2);
  static_assert(sizeof(i32) == 4);
  static_assert(sizeof(i64) == 8);
  static_assert(sizeof(u8) == 1);
  static_assert(sizeof(u16) == 2);
  static_assert(sizeof(u32) == 4);
  static_assert(sizeof(u64) == 8);
  static_assert(sizeof(f32) == 4);
  static_assert(sizeof(f64) == 8);
  static_assert(sizeof(uptr) == sizeof(void *));
  static_assert(sizeof(usize) == sizeof(void *));

  enum class platform : i8
  {
    linux_like,
    macos_like,
    windows_like,
    other_unix_like
  };

  static constexpr platform const current_platform{
#if defined(_WIN32) || defined(__CYGWIN__)
    platform::windows_like
#elif defined(__linux__)
    platform::linux_like
#elif defined(__APPLE__) && defined(__MACH__)
    platform::macos_like
#elif defined(unix) || defined(__unix__) || defined(__unix)
    platform::other_unix_like
#else
  #error Unsupported environment.
#endif
  };
}

namespace jank
{
  using jtl::i8;
  using jtl::u8;

  using jtl::i16;
  using jtl::u16;

  using jtl::i32;
  using jtl::u32;

  using jtl::i64;
  using jtl::u64;

  using jtl::f32;
  using jtl::f64;

  using jtl::uptr;
  using jtl::usize;
  using jtl::uhash;
  using jtl::nullptr_t;
}
