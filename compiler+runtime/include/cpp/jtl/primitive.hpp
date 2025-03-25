#pragma once

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
}
