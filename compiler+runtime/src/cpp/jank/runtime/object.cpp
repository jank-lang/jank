#include <jank/runtime/object.hpp>
#include <jank/runtime/core/equal.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/hash.hpp>

namespace jank::runtime
{
  bool very_equal_to::operator()(object_ref const lhs, object_ref const rhs) const noexcept
  {
    if(lhs->type != rhs->type)
    {
      return false;
    }
    return equal(lhs, rhs);
  }

  bool operator==(object * const lhs, object_ref const rhs)
  {
    return equal(lhs, rhs);
  }

  bool operator!=(object * const lhs, object_ref const rhs)
  {
    return !equal(lhs, rhs);
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

  native_bool
  // NOLINTNEXTLINE(bugprone-exception-escape): TODO: Sort this out.
  equal_to<object_ref>::operator()(object_ref const lhs, object_ref const rhs) const noexcept
  {
    if(!lhs)
    {
      return !rhs;
    }
    else if(!rhs)
    {
      return false;
    }

    return visit_object([&](auto const typed_lhs) { return typed_lhs->equal(*rhs); }, lhs);
  }
}
