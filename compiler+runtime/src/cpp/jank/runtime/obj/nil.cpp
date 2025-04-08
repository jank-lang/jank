#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/obj/persistent_array_map.hpp>
#include <jank/runtime/obj/cons.hpp>

namespace jank::runtime::obj
{
  nil_ref nil::nil_const()
  {
    //return reinterpret_cast<nil*>(const_cast<object*>(&jank::runtime::detail::nil_const));
    //return reinterpret_cast<nil*>(const_cast<object*>(jank_nil_const));
    static nil n;
    return &n;
  }

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

  native_integer nil::compare(object const &o) const
  {
    return (o.type == object_type::nil ? 0 : -1);
  }

  native_integer nil::compare(nil const &) const
  {
    return 0;
  }

  object_ptr nil::get(object_ptr const)
  {
    return &base;
  }

  object_ptr nil::get(object_ptr const, object_ptr const fallback)
  {
    return fallback;
  }

  object_ptr nil::get_entry(object_ptr)
  {
    return &base;
  }

  native_bool nil::contains(object_ptr) const
  {
    return false;
  }

  persistent_array_map_ref nil::assoc(object_ptr const key, object_ptr const val) const
  {
    return persistent_array_map::create_unique(key, val);
  }

  nil_ref nil::dissoc(object_ptr const) const
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

/* NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) */
jank::runtime::object *jank_nil_const{ jank::runtime::obj::nil::nil_const() };
