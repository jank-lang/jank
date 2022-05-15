#include <jank/runtime/object.hpp>
#include <jank/runtime/number.hpp>

namespace jank::runtime
{
  object_ptr JANK_NIL{ make_box<nil>() };

  detail::boolean_type object::equal(object_ptr const &rhs) const
  { return rhs && equal(*rhs); }

  detail::boolean_type nil::equal(object const &o) const
  {
    auto const *b(o.as_boolean());
    return b != nullptr;
  }
  detail::string_type nil::to_string() const
  /* TODO: Optimize. */
  { return "nil"; }
  detail::integer_type nil::to_hash() const
  { return 0; }
  nil const* nil::as_nil() const
  { return this; }
}
