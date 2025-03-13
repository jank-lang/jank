#include <jank/runtime/obj/volatile.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  volatile_::volatile_(object_ptr const o)
    : val{ o }
  {
    assert(val);
  }

  native_bool volatile_::equal(object const &o) const
  {
    return &o == &base;
  }

  native_persistent_string volatile_::to_string() const
  {
    util::string_builder buff;
    to_string(buff);
    return buff.release();
  }

  void volatile_::to_string(util::string_builder &buff) const
  {
    util::format_to(buff, "{}@{}", object_type_str(base.type), &base);
  }

  native_persistent_string volatile_::to_code_string() const
  {
    return to_string();
  }

  native_hash volatile_::to_hash() const
  {
    return static_cast<native_hash>(reinterpret_cast<uintptr_t>(this));
  }

  object_ptr volatile_::deref() const
  {
    return val;
  }

  object_ptr volatile_::reset(object_ptr const o)
  {
    val = o;
    assert(val);
    return val;
  }
}
