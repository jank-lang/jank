#include <jank/runtime/object.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/hash.hpp>

namespace jank::runtime
{
  bool very_equal_to::operator()(object_ptr const lhs, object_ptr const rhs) const noexcept
  {
    if(lhs->type != rhs->type)
    {
      return false;
    }
    return equal(lhs, rhs);
  }
}

namespace std
{
  using namespace jank;
  using namespace jank::runtime;

  size_t hash<object_ptr>::operator()(object_ptr const o) const noexcept
  {
    return jank::hash::visit(o);
  }

  size_t hash<object>::operator()(object const &o) const noexcept
  {
    return jank::hash::visit(const_cast<runtime::object *>(&o));
  }

  native_bool
  // NOLINTNEXTLINE(bugprone-exception-escape): TODO: Sort this out.
  equal_to<object_ptr>::operator()(object_ptr const lhs, object_ptr const rhs) const noexcept
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
