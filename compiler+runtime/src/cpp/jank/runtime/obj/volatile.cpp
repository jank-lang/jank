#include <jank/runtime/obj/volatile.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  volatile_::volatile_(object_ref const o)
    : val{ o }
  {
    jank_debug_assert(val.is_some());
  }

  bool volatile_::equal(object const &o) const
  {
    return &o == &base;
  }

  jtl::immutable_string volatile_::to_string() const
  {
    util::string_builder buff;
    to_string(buff);
    return buff.release();
  }

  void volatile_::to_string(util::string_builder &buff) const
  {
    util::format_to(buff, "{}@{}", object_type_str(base.type), &base);
  }

  jtl::immutable_string volatile_::to_code_string() const
  {
    return to_string();
  }

  uhash volatile_::to_hash() const
  {
    return static_cast<uhash>(reinterpret_cast<uintptr_t>(this));
  }

  object_ref volatile_::deref() const
  {
    return val;
  }

  object_ref volatile_::reset(object_ref const o)
  {
    val = o;
    jank_debug_assert(val.is_some());
    return val;
  }
}
