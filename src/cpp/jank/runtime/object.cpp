#include <fmt/compile.h>

#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/obj/string.hpp>
#include <jank/runtime/obj/list.hpp>

namespace jank::runtime
{
  /* TODO: Replace these with non-static values. */
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
  obj::nil_ptr JANK_NIL{ jank::make_box<obj::nil>() };

  namespace obj
  {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    boolean_ptr JANK_TRUE{ jank::make_box<boolean>(true) };
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    boolean_ptr JANK_FALSE{ jank::make_box<boolean>(false) };
  }

  native_bool object::equal(object const &rhs) const
  { return to_hash() == rhs.to_hash(); }

  native_bool object::equal(object_ptr rhs) const
  { return rhs && equal(*rhs); }

  void object::to_string(fmt::memory_buffer &buffer) const
  { fmt::format_to(std::back_inserter(buffer), FMT_COMPILE("{}"), to_string()); }

  bool object::operator <(object const &rhs) const
  { return to_hash() < rhs.to_hash(); }

  native_bool obj::nil::equal(object const &o) const
  {
    auto const *n(o.as_nil());
    return n != nullptr;
  }
  native_string obj::nil::to_string() const
  /* TODO: Optimize. */
  { return "nil"; }
  native_integer obj::nil::to_hash() const
  { return 0; }
  obj::nil const* obj::nil::as_nil() const
  { return this; }

  behavior::associatively_readable const* obj::nil::as_associatively_readable() const
  { return this; }
  object_ptr obj::nil::get(object_ptr const) const
  { return JANK_NIL; }
  object_ptr obj::nil::get(object_ptr const, object_ptr const fallback) const
  { return fallback; }

  std::ostream& operator<<(std::ostream &os, object const &o)
  /* TODO: Optimize this by using virtual dispatch to write into the stream, rather than allocating a string. */
  { return os << o.to_string(); }
}
