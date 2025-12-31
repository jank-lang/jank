#include <jank/runtime/object.hpp>
#include <jank/runtime/core/equal.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/hash.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::runtime
{
  object::object(object const &rhs) noexcept
    : type{ rhs.type }
  {
  }

  object::object(object &&rhs) noexcept
    : type{ rhs.type }
  {
  }

  object::object(object_type const type) noexcept
    : type{ type }
  {
  }

  object &object::operator=(object const &rhs) noexcept
  {
    if(this == &rhs)
    {
      return *this;
    }
    type = rhs.type;
    return *this;
  }

  object &object::operator=(object &&rhs) noexcept
  {
    if(this == &rhs)
    {
      return *this;
    }
    type = rhs.type;
    return *this;
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
  size_t
  hash<jank::runtime::object_ref>::operator()(jank::runtime::object_ref const o) const noexcept
  {
    return jank::hash::visit(o.data);
  }

  size_t hash<jank::runtime::object>::operator()(jank::runtime::object const &o) const noexcept
  {
    return jank::hash::visit(const_cast<jank::runtime::object *>(&o));
  }

  bool
  // NOLINTNEXTLINE(bugprone-exception-escape): TODO: Sort this out.
  equal_to<jank::runtime::object_ref>::operator()(
    jank::runtime::object_ref const lhs,
    jank::runtime::object_ref const rhs) const noexcept
  {
    return jank::runtime::equal(lhs, rhs);
  }
}
