#include <fmt/compile.h>

#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/obj/persistent_array_map.hpp>
#include <jank/runtime/obj/cons.hpp>

namespace jank::runtime::obj
{
  nil_ptr nil::nil_const()
  {
    static nil r{};
    return &r;
  }

  native_bool nil::equal(object const &o) const
  {
    return &o == &base;
  }

  native_persistent_string const &nil::to_string() const
  {
    static native_persistent_string const s{ "nil" };
    return s;
  }

  native_persistent_string const &nil::to_code_string() const
  {
    return to_string();
  }

  void nil::to_string(util::string_builder &buff) const
  {
    fmt::format_to(std::back_inserter(buff), "nil");
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

  persistent_array_map_ptr nil::assoc(object_ptr const key, object_ptr const val) const
  {
    return persistent_array_map::create_unique(key, val);
  }

  nil_ptr nil::dissoc(object_ptr const) const
  {
    return this;
  }

  nil_ptr nil::seq()
  {
    return nullptr;
  }

  nil_ptr nil::fresh_seq() const
  {
    return nullptr;
  }

  nil_ptr nil::first() const
  {
    return this;
  }

  nil_ptr nil::next() const
  {
    return nullptr;
  }

  nil_ptr nil::next_in_place()
  {
    return nullptr;
  }
}
