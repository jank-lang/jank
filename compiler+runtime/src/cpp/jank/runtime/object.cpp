#include <jank/runtime/detail/object_util.hpp>
#include <jank/hash.hpp>

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

  // NOLINTNEXTLINE(bugprone-exception-escape): TODO: Sort this out.
  native_bool
  equal_to<object_ptr>::operator()(object_ptr const lhs, object_ptr const rhs) const noexcept
  {
    if(!lhs)
    {
      return !rhs;
    }
    else if(!rhs)
    {
      return !lhs;
    }

    return visit_object([&](auto const typed_lhs) { return typed_lhs->equal(*rhs); }, lhs);
  }
}
