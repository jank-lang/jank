#include <jank/runtime/object.hpp>
#include <jank/runtime/type/number.hpp>

namespace jank::runtime
{
  /* XXX: Any static helpers stay in here, to ensure they're all created deterministically. */
  object_ptr JANK_NIL{ make_box<type::nil>() };

  namespace type
  {
    object_ptr JANK_TRUE{ make_box<boolean>(true) };
    object_ptr JANK_FALSE{ make_box<boolean>(false) };
  }

  detail::boolean_type object::equal(object_ptr const &rhs) const
  { return rhs && equal(*rhs); }

  detail::boolean_type type::nil::equal(object const &o) const
  {
    auto const *b(o.as_boolean());
    return b != nullptr;
  }
  detail::string_type type::nil::to_string() const
  /* TODO: Optimize. */
  { return "nil"; }
  detail::integer_type type::nil::to_hash() const
  { return 0; }
  type::nil const* type::nil::as_nil() const
  { return this; }
}
