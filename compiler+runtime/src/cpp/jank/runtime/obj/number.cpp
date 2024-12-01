#include <fmt/compile.h>

#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/visit.hpp>

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
  {
  }

  native_bool obj::boolean::equal(object const &o) const
  {
    if(o.type != object_type::boolean)
    {
      return false;
    }

    auto const b(expect_object<obj::boolean>(&o));
    return data == b->data;
  }

  static void to_string_impl(bool const data, fmt::memory_buffer &buff)
  {
    format_to(std::back_inserter(buff), FMT_COMPILE("{}"), data ? "true" : "false");
  }

  void obj::boolean::to_string(fmt::memory_buffer &buff) const
  {
    to_string_impl(data, buff);
  }

  native_persistent_string obj::boolean::to_string() const
  {
    fmt::memory_buffer buff;
    to_string_impl(data, buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  native_persistent_string obj::boolean::to_code_string() const
  {
    return to_string();
  }

  native_hash obj::boolean::to_hash() const
  {
    return data ? 1231 : 1237;
  }

  native_integer obj::boolean::compare(object const &o) const
  {
    return visit_number_like(
      [this](auto const typed_o) -> native_integer {
        return (data > typed_o->data) - (data < typed_o->data);
      },
      [&]() -> native_integer {
        throw std::runtime_error{ fmt::format("not comparable: {}", runtime::to_string(&o)) };
      },
      &o);
  }

  native_integer obj::boolean::compare(obj::boolean const &o) const
  {
    return (data > o.data) - (data < o.data);
  }

  /***** integer *****/
  obj::integer::static_object(native_integer const d)
    : data{ d }
  {
  }

  native_bool obj::integer::equal(object const &o) const
  {
    if(o.type != object_type::integer)
    {
      return false;
    }

    auto const i(expect_object<obj::integer>(&o));
    return data == i->data;
  }

  native_persistent_string obj::integer::to_string() const
  {
    return fmt::format(FMT_COMPILE("{}"), data);
  }

  void obj::integer::to_string(fmt::memory_buffer &buff) const
  {
    fmt::format_to(std::back_inserter(buff), FMT_COMPILE("{}"), data);
  }

  native_persistent_string obj::integer::to_code_string() const
  {
    return to_string();
  }

  native_hash obj::integer::to_hash() const
  {
    return hash::integer(data);
  }

  native_integer obj::integer::compare(object const &o) const
  {
    return visit_number_like(
      [this](auto const typed_o) -> native_integer {
        return (data > typed_o->data) - (data < typed_o->data);
      },
      [&]() -> native_integer {
        throw std::runtime_error{ fmt::format("not comparable: {}", runtime::to_string(&o)) };
      },
      &o);
  }

  native_integer obj::integer::compare(obj::integer const &o) const
  {
    return (data > o.data) - (data < o.data);
  }

  native_integer obj::integer::to_integer() const
  {
    return data;
  }

  native_real obj::integer::to_real() const
  {
    return static_cast<native_real>(data);
  }

  /***** real *****/
  obj::real::static_object(native_real const d)
    : data{ d }
  {
  }

  native_bool obj::real::equal(object const &o) const
  {
    if(o.type != object_type::real)
    {
      return false;
    }

    auto const r(expect_object<obj::real>(&o));
    std::hash<native_real> const hasher{};
    return hasher(data) == hasher(r->data);
  }

  native_persistent_string obj::real::to_string() const
  {
    return fmt::format(FMT_COMPILE("{}"), data);
  }

  void obj::real::to_string(fmt::memory_buffer &buff) const
  {
    fmt::format_to(std::back_inserter(buff), FMT_COMPILE("{}"), data);
  }

  native_persistent_string obj::real::to_code_string() const
  {
    return to_string();
  }

  native_hash obj::real::to_hash() const
  {
    return hash::real(data);
  }

  native_integer obj::real::compare(object const &o) const
  {
    return visit_number_like(
      [this](auto const typed_o) -> native_integer {
        return (data > typed_o->data) - (data < typed_o->data);
      },
      [&]() -> native_integer {
        throw std::runtime_error{ fmt::format("not comparable: {}", runtime::to_string(&o)) };
      },
      &o);
  }

  native_integer obj::real::compare(obj::real const &o) const
  {
    return (data > o.data) - (data < o.data);
  }

  native_integer obj::real::to_integer() const
  {
    return static_cast<native_integer>(data);
  }

  native_real obj::real::to_real() const
  {
    return data;
  }
}
