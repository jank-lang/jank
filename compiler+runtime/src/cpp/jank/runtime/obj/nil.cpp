#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/obj/persistent_array_map.hpp>
#include <jank/runtime/obj/cons.hpp>

namespace jank::runtime::obj
{
  native_bool nil::equal(object const &o) const
  {
    return o.type == obj_type;
  }

  jtl::immutable_string const &nil::to_string() const
  {
    static jtl::immutable_string const s{ "nil" };
    return s;
  }

  jtl::immutable_string const &nil::to_code_string() const
  {
    return to_string();
  }

  void nil::to_string(util::string_builder &buff) const
  {
    buff("nil");
  }

  native_hash nil::to_hash() const
  {
    return 0;
  }

  i64 nil::compare(object const &o) const
  {
    return (o.type == object_type::nil ? 0 : -1);
  }

  i64 nil::compare(nil const &) const
  {
    return 0;
  }

  object_ref nil::get(object_ref const)
  {
    return &base;
  }

  object_ref nil::get(object_ref const, object_ref const fallback)
  {
    return fallback;
  }

  object_ref nil::get_entry(object_ref)
  {
    return &base;
  }

  native_bool nil::contains(object_ref) const
  {
    return false;
  }

  persistent_array_map_ref nil::assoc(object_ref const key, object_ref const val) const
  {
    return persistent_array_map::create_unique(key, val);
  }

  nil_ref nil::dissoc(object_ref const) const
  {
    return this;
  }

  nil_ref nil::seq()
  {
    return this;
  }

  nil_ref nil::fresh_seq() const
  {
    return this;
  }

  nil_ref nil::first() const
  {
    return this;
  }

  nil_ref nil::next() const
  {
    return this;
  }

  nil_ref nil::next_in_place()
  {
    return this;
  }
}

namespace jank::runtime
{
  bool operator==(object * const lhs, obj::nil_ref)
  {
    return lhs->type == object_type::nil;
  }

  bool operator!=(object * const lhs, obj::nil_ref)
  {
    return lhs->type != object_type::nil;
  }

  static obj::nil_ref nil_const()
  {
    static obj::nil n;
    return &n;
  }

  /* NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) */
  obj::nil_ref jank_nil{ nil_const() };

  namespace detail
  {
    /* NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) */
    obj::nil *jank_nil_ptr{ nil_const().data };
  }
}
