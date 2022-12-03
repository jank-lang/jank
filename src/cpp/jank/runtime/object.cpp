#include <jank/runtime/object.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/obj/string.hpp>
#include <jank/runtime/obj/list.hpp>

namespace jank::runtime
{
  /* TODO: Replace these with non-static values. */
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
  object_ptr JANK_NIL{ make_box<obj::nil>() };

  namespace obj
  {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    object_ptr JANK_TRUE{ make_box<boolean>(true) };
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
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

  object_ptr make_box(std::nullptr_t const &)
  { return JANK_NIL; }
  object_ptr make_box(bool const b)
  { return b ? obj::JANK_TRUE : obj::JANK_FALSE; }
  object_ptr make_box(int const i)
  { return make_box<obj::integer>(detail::integer_type{ i }); }
  object_ptr make_box(detail::integer_type const i)
  { return make_box<obj::integer>(i); }
  object_ptr make_box(detail::real_type const r)
  { return make_box<obj::real>(r); }
  object_ptr make_box(detail::string_type const &s)
  { return make_box<obj::string>(s); }
  object_ptr make_box(std::string_view const &s)
  { return make_box<obj::string>(s); }
  object_ptr make_box(detail::list_type const &l)
  { return make_box<obj::list>(l); }
}
