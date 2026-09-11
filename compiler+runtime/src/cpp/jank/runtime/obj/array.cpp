#include <jank/runtime/obj/array.hpp>

namespace jank::runtime::obj
{
  template struct array<bool>;
  template struct array<char>;
  template struct array<u8>;
  template struct array<i8>;
  template struct array<u16>;
  template struct array<i16>;
  template struct array<u32>;
  template struct array<i32>;
  template struct array<u64>;
  template struct array<i64>;
  template struct array<f32>;
  template struct array<f64>;
  template struct array<object_ref>;
}
