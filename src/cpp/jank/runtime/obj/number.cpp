#include <iostream>
#include <random>
#include <cmath>

#include <fmt/compile.h>

#include <jank/runtime/obj/number.hpp>

namespace jank::runtime
{
  /***** boolean *****/
  obj::boolean_ptr obj::boolean::true_const()
  {
    static obj::boolean r{ true };
    return &r;
  }
  obj::boolean_ptr obj::boolean::false_const()
  {
    static obj::boolean r{ false };
    return &r;
  }

  obj::boolean::static_object(native_bool const d)
    : data{ d }
  { }

  native_bool obj::boolean::equal(object const &o) const
  {
    if(o.type != object_type::boolean)
    { return false; }

    auto const b(expect_object<obj::boolean>(&o));
    return data == b->data;
  }

  void to_string_impl(bool const data, fmt::memory_buffer &buff)
  { format_to(std::back_inserter(buff), FMT_COMPILE("{}"), data ? "true" : "false"); }

  void obj::boolean::to_string(fmt::memory_buffer &buff) const
  { return to_string_impl(data, buff); }

  native_string obj::boolean::to_string() const
  {
    fmt::memory_buffer buff;
    to_string_impl(data, buff);
    return native_string{ buff.data(), buff.size() };
  }

  native_integer obj::boolean::to_hash() const
  /* TODO: Do what Clojure does here instead. */
  { return data ? 1 : 0; }

  /***** integer *****/
  obj::integer::static_object(native_integer const d)
    : data{ d }
  { }

  native_bool obj::integer::equal(object const &o) const
  {
    if(o.type != object_type::integer)
    { return false; }

    auto const i(expect_object<obj::integer>(&o));
    return data == i->data;
  }

  native_string obj::integer::to_string() const
  { return fmt::format(FMT_COMPILE("{}"), data); }

  void obj::integer::to_string(fmt::memory_buffer &buff) const
  { fmt::format_to(std::back_inserter(buff), FMT_COMPILE("{}"), data); }

  native_integer obj::integer::to_hash() const
  { return data; }

  native_integer obj::integer::to_integer() const
  { return data; }

  native_real obj::integer::to_real() const
  { return data; }

  /***** real *****/
  obj::real::static_object(native_real const d)
    : data{ d }
  { }

  native_bool obj::real::equal(object const &o) const
  {
    if(o.type != object_type::real)
    { return false; }

    auto const r(expect_object<obj::real>(&o));
    std::hash<native_real> hasher{};
    return hasher(data) == hasher(r->data);
  }

  native_string obj::real::to_string() const
  { return fmt::format(FMT_COMPILE("{}"), data); }

  void obj::real::to_string(fmt::memory_buffer &buff) const
  { fmt::format_to(std::back_inserter(buff), FMT_COMPILE("{}"), data); }

  native_integer obj::real::to_hash() const
  { return static_cast<native_integer>(data); }

  native_integer obj::real::to_integer() const
  { return static_cast<native_integer>(data); }

  native_real obj::real::to_real() const
  { return data; }
}
