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
