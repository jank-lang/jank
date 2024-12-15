#include <fmt/format.h>

#include <jank/native_persistent_string/fmt.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/visit.hpp>

namespace jank::runtime::obj
{
  /***** boolean *****/
  boolean_ptr boolean::true_const()
  {
    static boolean r{ true };
    return &r;
  }

  boolean_ptr boolean::false_const()
  {
    static boolean r{ false };
    return &r;
  }

  boolean::boolean(native_bool const d)
    : data{ d }
  {
  }

  native_bool boolean::equal(object const &o) const
  {
    if(o.type != object_type::boolean)
    {
      return false;
    }

    auto const b(expect_object<boolean>(&o));
    return data == b->data;
  }

  void boolean::to_string(util::string_builder &buff) const
  {
    buff(data);
  }

  native_persistent_string boolean::to_string() const
  {
    util::string_builder buff;
    buff(data);
    return buff.release();
  }

  native_persistent_string boolean::to_code_string() const
  {
    return to_string();
  }

  native_hash boolean::to_hash() const
  {
    return data ? 1231 : 1237;
  }

  native_integer boolean::compare(object const &o) const
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

  native_integer boolean::compare(boolean const &o) const
  {
    return (data > o.data) - (data < o.data);
  }

  /***** integer *****/
  integer::integer(native_integer const d)
    : data{ d }
  {
  }

  native_bool integer::equal(object const &o) const
  {
    if(o.type != object_type::integer)
    {
      return false;
    }

    auto const i(expect_object<integer>(&o));
    return data == i->data;
  }

  native_persistent_string integer::to_string() const
  {
    util::string_builder sb;
    return sb(data).release();
  }

  void integer::to_string(util::string_builder &buff) const
  {
    buff(data);
  }

  native_persistent_string integer::to_code_string() const
  {
    return to_string();
  }

  native_hash integer::to_hash() const
  {
    return hash::integer(data);
  }

  native_integer integer::compare(object const &o) const
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

  native_integer integer::compare(integer const &o) const
  {
    return (data > o.data) - (data < o.data);
  }

  native_integer integer::to_integer() const
  {
    return data;
  }

  native_real integer::to_real() const
  {
    return static_cast<native_real>(data);
  }

  /***** real *****/
  real::real(native_real const d)
    : data{ d }
  {
  }

  native_bool real::equal(object const &o) const
  {
    if(o.type != object_type::real)
    {
      return false;
    }

    auto const r(expect_object<real>(&o));
    std::hash<native_real> const hasher{};
    return hasher(data) == hasher(r->data);
  }

  native_persistent_string real::to_string() const
  {
    util::string_builder sb;
    return sb(data).release();
  }

  void real::to_string(util::string_builder &buff) const
  {
    buff(data);
  }

  native_persistent_string real::to_code_string() const
  {
    return to_string();
  }

  native_hash real::to_hash() const
  {
    return hash::real(data);
  }

  native_integer real::compare(object const &o) const
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

  native_integer real::compare(real const &o) const
  {
    return (data > o.data) - (data < o.data);
  }

  native_integer real::to_integer() const
  {
    return static_cast<native_integer>(data);
  }

  native_real real::to_real() const
  {
    return data;
  }
}
