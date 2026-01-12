#include <jank/runtime/obj/big_decimal.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime
{
  native_big_decimal operator+(native_big_decimal const &l, native_big_integer const &r)
  {
    return l + native_big_decimal(r.str());
  }

  native_big_decimal operator-(native_big_decimal const &l, native_big_integer const &r)
  {
    return l - native_big_decimal(r.str());
  }

  native_big_decimal operator*(native_big_decimal const &l, native_big_integer const &r)
  {
    return l * native_big_decimal(r.str());
  }

  native_big_decimal operator/(native_big_decimal const &l, native_big_integer const &r)
  {
    return l / native_big_decimal(r.str());
  }

  native_big_decimal operator+(native_big_integer const &l, native_big_decimal const &r)
  {
    return native_big_decimal(l.str()) + r;
  }

  native_big_decimal operator-(native_big_integer const &l, native_big_decimal const &r)
  {
    return native_big_decimal(l.str()) - r;
  }

  native_big_decimal operator*(native_big_integer const &l, native_big_decimal const &r)
  {
    return native_big_decimal(l.str()) * r;
  }

  native_big_decimal operator/(native_big_integer const &l, native_big_decimal const &r)
  {
    return native_big_decimal(l.str()) / r;
  }

  bool operator==(native_big_decimal const &l, native_big_integer const &r)
  {
    native_big_decimal const r_as_decimal(r.str());
    return abs(l - r_as_decimal) < std::numeric_limits<native_big_decimal>::epsilon();
  }

  bool operator!=(native_big_decimal const &l, native_big_integer const &r)
  {
    return !(l == r);
  }

  bool operator==(native_big_integer const &l, native_big_decimal const &r)
  {
    return r == l;
  }

  bool operator!=(native_big_integer const &l, native_big_decimal const &r)
  {
    return !(l == r);
  }

  bool operator>(native_big_decimal const &l, native_big_integer const &r)
  {
    return l > native_big_decimal(r.str());
  }

  bool operator>(native_big_integer const &l, native_big_decimal const &r)
  {
    return native_big_decimal(l.str()) > r;
  }

  bool operator<(native_big_decimal const &l, native_big_integer const &r)
  {
    return l < native_big_decimal(r.str());
  }

  bool operator<(native_big_integer const &l, native_big_decimal const &r)
  {
    return native_big_decimal(l.str()) < r;
  }

  bool operator>=(native_big_decimal const &l, native_big_integer const &r)
  {
    return l >= native_big_decimal(r.str());
  }

  bool operator>=(native_big_integer const &l, native_big_decimal const &r)
  {
    return native_big_decimal(l.str()) >= r;
  }

  bool operator<=(native_big_decimal const &l, native_big_integer const &r)
  {
    return l <= native_big_decimal(r.str());
  }

  bool operator<=(native_big_integer const &l, native_big_decimal const &r)
  {
    return native_big_decimal(l.str()) <= r;
  }
}

namespace jank::runtime::obj
{
  big_decimal::big_decimal(native_big_decimal const &val)
    : data{ val }
  {
  }

  big_decimal::big_decimal(native_big_decimal &&val)
    : data{ std::move(val) }
  {
  }

  big_decimal::big_decimal(jtl::immutable_string const &val)
    : data{ val.c_str() }
  {
  }

  big_decimal::big_decimal(native_big_integer const &val)
    : data{ val.real() }
  {
  }

  big_decimal::big_decimal(ratio const &val)
    : data(native_big_decimal(val.data.numerator) / val.data.denominator)
  {
  }

  bool big_decimal::equal(object const &o) const
  {
    return visit_number_like(
      [this](auto const typed_o) -> bool {
        return abs(data - typed_o->data) < std::numeric_limits<f64>::epsilon();
      },
      [&]() -> bool { return false; },
      &o);
  }

  jtl::immutable_string big_decimal::to_string() const
  {
    return data.str();
  }

  void big_decimal::to_string(jtl::string_builder &buff) const
  {
    buff(data.str());
  }

  jtl::immutable_string big_decimal::to_code_string() const
  {
    return to_string() + 'M';
  }

  uhash big_decimal::to_hash() const
  {
    return std::hash<native_big_decimal>{}(data);
  }

  i64 big_decimal::compare(object const &o) const
  {
    return visit_number_like(
      [this](auto const typed_o) -> i64 { return (data > typed_o->data) - (data < typed_o->data); },
      [&]() -> i64 {
        throw std::runtime_error{ util::format("not comparable: {}", runtime::to_string(&o)) };
      },
      &o);
  }

  i64 big_decimal::to_integer() const
  {
    return data.convert_to<i64>();
  }

  f64 big_decimal::to_real() const
  {
    return data.convert_to<f64>();
  }

  object_ref big_decimal::create(jtl::immutable_string const &val)
  {
    return make_box<big_decimal>(val).erase();
  }

}
