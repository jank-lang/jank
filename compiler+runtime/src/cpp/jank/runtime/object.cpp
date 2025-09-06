#include <jank/runtime/object.hpp>
#include <jank/runtime/core/equal.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/hash.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime
{
  object::object(object_type const t)
    : type{ t }
  {
  }

  bool object::equal(object const &o)
  {
    return this == &o;
  }

  jtl::immutable_string object::to_string()
  {
    jtl::string_builder buff;
    to_string(buff);
    return buff.release();
  }

  void object::to_string(jtl::string_builder &buff)
  {
    util::format_to(buff, "{}@{}", object_type_str(type), this);
  }

  jtl::immutable_string object::to_code_string()
  {
    return to_string();
  }

  uhash object::to_hash()
  {
    return static_cast<uhash>(reinterpret_cast<uintptr_t>(this));
  }

  bool very_equal_to::operator()(object_ref const lhs, object_ref const rhs) const noexcept
  {
    if(lhs->type != rhs->type)
    {
      return false;
    }
    return equal(lhs, rhs);
  }

  bool operator==(object const * const lhs, object_ref const rhs)
  {
    return lhs == rhs.data;
  }

  bool operator!=(object const * const lhs, object_ref const rhs)
  {
    return lhs != rhs.data;
  }
}

namespace std
{
  using namespace jank;
  using namespace jank::runtime;

  size_t hash<object_ref>::operator()(object_ref const o) const noexcept
  {
    return jank::hash::visit(o.data);
  }

  size_t hash<object>::operator()(object const &o) const noexcept
  {
    return jank::hash::visit(const_cast<runtime::object *>(&o));
  }

  bool
  // NOLINTNEXTLINE(bugprone-exception-escape): TODO: Sort this out.
  equal_to<object_ref>::operator()(object_ref const lhs, object_ref const rhs) const noexcept
  {
    return equal(lhs, rhs);
  }
}
