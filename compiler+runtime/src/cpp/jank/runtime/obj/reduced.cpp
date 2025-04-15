#include <jank/runtime/obj/reduced.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  reduced::reduced(object_ref const o)
    : val{ o }
  {
    jank_debug_assert(val);
  }

  native_bool reduced::equal(object const &o) const
  {
    return &o == &base;
  }

  jtl::immutable_string reduced::to_string() const
  {
    util::string_builder buff;
    to_string(buff);
    return buff.release();
  }

  void reduced::to_string(util::string_builder &buff) const
  {
    util::format_to(buff, "{}@{}", object_type_str(base.type), &base);
  }

  jtl::immutable_string reduced::to_code_string() const
  {
    return to_string();
  }

  native_hash reduced::to_hash() const
  {
    return static_cast<native_hash>(reinterpret_cast<uintptr_t>(this));
  }

  object_ref reduced::deref() const
  {
    return val;
  }
}
