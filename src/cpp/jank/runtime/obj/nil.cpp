#include <jank/runtime/obj/nil.hpp>

namespace jank::runtime
{
  obj::nil_ptr obj::nil::nil_const()
  {
    static obj::nil r{};
    return &r;
  }

  obj::nil::static_object(object &&base)
    : base{ std::move(base) }
  {
  }

  native_bool obj::nil::equal(object const &o) const
  {
    return &o == &base;
  }

  native_persistent_string const &obj::nil::to_string() const
  {
    static native_persistent_string s{ "nil" };
    return s;
  }

  void obj::nil::to_string(fmt::memory_buffer &buff) const
  {
    fmt::format_to(std::back_inserter(buff), "nil");
  }

  native_integer obj::nil::to_hash() const
  {
    return 0;
  }

  object_ptr obj::nil::get(object_ptr const)
  {
    return &base;
  }

  object_ptr obj::nil::get(object_ptr const, object_ptr const fallback)
  {
    return fallback;
  }

  object_ptr obj::nil::get_entry(object_ptr)
  {
    return &base;
  }

  native_bool obj::nil::contains(object_ptr) const
  {
    return false;
  }

  obj::nil_ptr obj::nil::seq()
  {
    return nullptr;
  }

  obj::nil_ptr obj::nil::fresh_seq() const
  {
    return nullptr;
  }

  obj::nil_ptr obj::nil::first() const
  {
    return this;
  }

  obj::nil_ptr obj::nil::next() const
  {
    return nullptr;
  }

  obj::nil_ptr obj::nil::next_in_place()
  {
    return nullptr;
  }

  obj::nil_ptr obj::nil::next_in_place_first()
  {
    return nullptr;
  }
}
