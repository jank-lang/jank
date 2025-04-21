#include <cmath>

#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  /***** boolean *****/
  boolean::boolean(bool const d)
    : data{ d }
  {
  }

  bool boolean::equal(object const &o) const
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

  jtl::immutable_string boolean::to_string() const
  {
    util::string_builder buff;
    buff(data);
    return buff.release();
  }

  jtl::immutable_string boolean::to_code_string() const
  {
    return to_string();
  }

  uhash boolean::to_hash() const
  {
    return data ? 1231 : 1237;
  }

  i64 boolean::compare(object const &o) const
  {
    return visit_number_like(
      [this](auto const typed_o) -> i64 {
        return (data > typed_o->data) - (data < typed_o->data);
      },
      [&]() -> i64 {
        throw std::runtime_error{ util::format("not comparable: {}", runtime::to_string(&o)) };
      },
      &o);
  }

  i64 boolean::compare(boolean const &o) const
  {
    return (data > o.data) - (data < o.data);
  }

  /***** integer *****/
  integer::integer(i64 const d)
    : data{ d }
  {
  }

  bool integer::equal(object const &o) const
  {
    if(o.type != object_type::integer)
    {
      return false;
    }

    auto const i(expect_object<integer>(&o));
    return data == i->data;
  }

  jtl::immutable_string integer::to_string() const
  {
    util::string_builder sb;
    return sb(data).release();
  }

  void integer::to_string(util::string_builder &buff) const
  {
    buff(data);
  }

  jtl::immutable_string integer::to_code_string() const
  {
    return to_string();
  }

  uhash integer::to_hash() const
  {
    return hash::integer(data);
  }

  i64 integer::compare(object const &o) const
  {
    return visit_number_like(
      [this](auto const typed_o) -> i64 {
        return (data > typed_o->data) - (data < typed_o->data);
      },
      [&]() -> i64 {
        throw std::runtime_error{ util::format("not comparable: {}", runtime::to_string(&o)) };
      },
      &o);
  }

  i64 integer::compare(integer const &o) const
  {
    return (data > o.data) - (data < o.data);
  }

  i64 integer::to_integer() const
  {
    return data;
  }

  f64 integer::to_real() const
  {
    return static_cast<f64>(data);
  }

  /***** real *****/
  real::real(f64 const d)
    : data{ d }
  {
  }

  bool real::equal(object const &o) const
  {
    if(o.type != object_type::real)
    {
      return false;
    }

    auto const r(expect_object<real>(&o));
    std::hash<f64> const hasher{};
    return hasher(data) == hasher(r->data);
  }

  jtl::immutable_string real::to_string() const
  {
    util::string_builder sb;
    to_string(sb);
    return sb.release();
  }

  void real::to_string(util::string_builder &buff) const
  {
    if(std::isinf(data))
    {
      if(data < 0)
      {
        buff("##-Inf");
      }
      else
      {
        buff("##Inf");
      }
    }
    else if(std::isnan(data))
    {
      buff("##NaN");
    }
    else
    {
      buff(data);
    }
  }

  jtl::immutable_string real::to_code_string() const
  {
    return to_string();
  }

  uhash real::to_hash() const
  {
    return hash::real(data);
  }

  i64 real::compare(object const &o) const
  {
    return visit_number_like(
      [this](auto const typed_o) -> i64 {
        return (data > typed_o->data) - (data < typed_o->data);
      },
      [&]() -> i64 {
        throw std::runtime_error{ util::format("not comparable: {}", runtime::to_string(&o)) };
      },
      &o);
  }

  i64 real::compare(real const &o) const
  {
    return (data > o.data) - (data < o.data);
  }

  i64 real::to_integer() const
  {
    return static_cast<i64>(data);
  }

  f64 real::to_real() const
  {
    return data;
  }
}

namespace jank::runtime
{
  static obj::boolean_ref true_const()
  {
    static obj::boolean r{ true };
    return &r;
  }

  static obj::boolean_ref false_const()
  {
    static obj::boolean r{ false };
    return &r;
  }

  /* NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) */
  obj::boolean_ref jank_true{ true_const() };
  /* NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) */
  obj::boolean_ref jank_false{ false_const() };
}
