#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/number.hpp>

namespace jank::runtime
{
  /* TODO: Replace these with non-static values. */
  object_ptr JANK_NIL{ make_box<obj::nil>() };

  namespace obj
  {
    object_ptr JANK_TRUE{ make_box<boolean>(true) };
    object_ptr JANK_FALSE{ make_box<boolean>(false) };
  }

  detail::boolean_type object::equal(object_ptr const &rhs) const
  { return rhs && equal(*rhs); }

  detail::boolean_type obj::nil::equal(object const &o) const
  {
    auto const *n(o.as_nil());
    return n != nullptr;
  }
  detail::string_type obj::nil::to_string() const
  /* TODO: Optimize. */
  { return "nil"; }
  detail::integer_type obj::nil::to_hash() const
  { return 0; }
  obj::nil const* obj::nil::as_nil() const
  { return this; }
}
