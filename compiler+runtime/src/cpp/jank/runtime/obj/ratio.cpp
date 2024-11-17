#include <jank/runtime/obj/ratio.hpp>
#include <fmt/compile.h>
#include <limits>

namespace jank::runtime
{

  constexpr native_real EPSILON = std::numeric_limits<native_real>::epsilon();

  obj::ratio_data::static_object(native_integer numerator, native_integer denominator)
    : numerator{ numerator }
    , denominator{ denominator }
  {
    if(denominator == 0)
    {
      throw std::invalid_argument("Ratio denominator cannot be zero.");
    }
    simplify();
  }

  object_ptr obj::ratio::create(native_integer const numerator, native_integer const denominator)
  {
    auto ratio_data = make_box<obj::ratio_data>(numerator, denominator);
    if(ratio_data->denominator == 1)
    {
      return make_box<obj::integer>(ratio_data->numerator);
    }
    auto r(make_box<obj::ratio>());
    r->data.numerator = ratio_data->numerator;
    r->data.denominator = ratio_data->denominator;
    return r;
  }

  native_real obj::ratio_data::to_real() const
  {
    return static_cast<native_real>(numerator) / denominator;
  }

  native_real obj::ratio::to_real() const
  {
    return data.to_real();
  }

  native_integer obj::ratio::to_integer() const
  {
    return data.numerator / data.denominator;
  }

  void obj::ratio_data::simplify()
  {
    int gcd = std::gcd(numerator, denominator);
    numerator /= gcd;
    denominator /= gcd;

    if(denominator < 0)
    {
      numerator = -1 * numerator;
      denominator = -1 * denominator;
    }
  }

  void obj::ratio::to_string(fmt::memory_buffer &buff) const
  {
    format_to(std::back_inserter(buff), FMT_COMPILE("{}/{}"), data.numerator, data.denominator);
  }

  native_persistent_string obj::ratio::to_string() const
  {
    fmt::memory_buffer buff;
    to_string(buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  native_persistent_string obj::ratio::to_code_string() const
  {
    return to_string();
  }

  native_hash obj::ratio::to_hash() const
  {
    return hash::combine(hash::integer(data.numerator), hash::integer(data.denominator));
  }

  native_bool obj::ratio::equal(object const &o) const
  {
    if(o.type == object_type::integer)
    {
      auto const i(expect_object<obj::integer>(&o));
      return this->data == i->data;
    }

    if(o.type == object_type::real)
    {
      auto const i(expect_object<obj::integer>(&o));
      return this->data == i->data;
    }

    if(o.type == object_type::ratio)
    {
      return this == expect_object<obj::ratio>(&o).data;
    }

    return false;
  }

  native_integer obj::ratio::compare(object const &o) const
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

  native_integer obj::ratio::compare(static_object const &o) const
  {
    return (data > o.data) - (data < o.data);
  }

  object_ptr operator+(obj::ratio_data const l, obj::ratio_data const r)
  {
    native_integer denom = l.denominator * r.denominator;
    native_integer num = l.numerator * r.denominator + r.numerator * l.denominator;
    return obj::ratio::create(num, denom);
  }

  obj::ratio_ptr operator+(obj::integer_ptr l, obj::ratio_data const r)
  {
    return expect_object<obj::ratio>(obj::ratio_data(l->data, 1LL) + r);
  }

  obj::ratio_ptr operator+(obj::ratio_data const l, obj::integer_ptr r)
  {
    return r + l;
  }

  native_real operator+(obj::real_ptr l, obj::ratio_data const r)
  {
    return l->data + r.to_real();
  }

  native_real operator+(obj::ratio_data const l, obj::real_ptr r)
  {
    return l.to_real() + r->data;
  }

  native_real operator+(obj::ratio_data const l, native_real r)
  {
    return l.to_real() + r;
  }

  native_real operator+(native_real l, obj::ratio_data const r)
  {
    return l + r.to_real();
  }

  obj::ratio_ptr operator+(obj::ratio_data const l, native_integer r)
  {
    return expect_object<obj::ratio>(
      obj::ratio::create(l.numerator + r * l.denominator, l.denominator));
  }

  obj::ratio_ptr operator+(native_integer l, obj::ratio_data const r)
  {
    return r + l;
  }

  object_ptr operator-(obj::ratio_data const l, obj::ratio_data const r)
  {
    native_integer denom = l.denominator * r.denominator;
    native_integer num = l.numerator * r.denominator - r.numerator * l.denominator;
    return obj::ratio::create(num, denom);
  }

  obj::ratio_ptr operator-(obj::integer_ptr l, obj::ratio_data const r)
  {
    return expect_object<obj::ratio>(
      obj::ratio::create(l->data * r.denominator - r.numerator, r.denominator));
  }

  obj::ratio_ptr operator-(obj::ratio_data const l, obj::integer_ptr r)
  {
    return expect_object<obj::ratio>(
      obj::ratio::create(l.numerator - l.denominator * r->data, l.denominator));
  }

  native_real operator-(obj::real_ptr l, obj::ratio_data const r)
  {
    return l->data - r.to_real();
  }

  native_real operator-(obj::ratio_data const l, obj::real_ptr r)
  {
    return l.to_real() - r->data;
  }

  native_real operator-(obj::ratio_data const l, native_real r)
  {
    return l.to_real() - r;
  }

  native_real operator-(native_real l, obj::ratio_data const r)
  {
    return l - r.to_real();
  }

  obj::ratio_ptr operator-(obj::ratio_data const l, native_integer r)
  {
    return expect_object<obj::ratio>(
      obj::ratio::create(l.numerator - r * l.denominator, l.denominator));
  }

  obj::ratio_ptr operator-(native_integer l, obj::ratio_data const r)
  {
    return expect_object<obj::ratio>(
      obj::ratio::create(l * r.denominator - r.denominator, r.denominator));
  }

  object_ptr operator*(obj::ratio_data const l, obj::ratio_data const r)
  {
    return obj::ratio::create(l.numerator * r.numerator, l.denominator * r.denominator);
  }

  object_ptr operator*(obj::integer_ptr l, obj::ratio_data const r)
  {
    return obj::ratio_data(l->data, 1LL) * r;
  }

  object_ptr operator*(obj::ratio_data const l, obj::integer_ptr r)
  {
    return l * obj::ratio_data(r->data, 1LL);
  }

  native_real operator*(obj::real_ptr l, obj::ratio_data const r)
  {
    return l->data * r.to_real();
  }

  native_real operator*(obj::ratio_data const l, obj::real_ptr r)
  {
    return l.to_real() * r->data;
  }

  native_real operator*(obj::ratio_data const l, native_real r)
  {
    return l.to_real() * r;
  }

  native_real operator*(native_real l, obj::ratio_data const r)
  {
    return l * r.to_real();
  }

  object_ptr operator*(obj::ratio_data const l, native_integer r)
  {
    return l * obj::ratio_data(r, 1LL);
  }

  object_ptr operator*(native_integer l, obj::ratio_data const r)
  {
    return r * l;
  }

  object_ptr operator/(obj::ratio_data const l, obj::ratio_data const r)
  {
    return obj::ratio::create(l.numerator * r.denominator, l.denominator * r.numerator);
  }

  object_ptr operator/(obj::integer_ptr l, obj::ratio_data const r)
  {
    return obj::ratio_data(l->data, 1LL) / r;
  }

  obj::ratio_ptr operator/(obj::ratio_data const l, obj::integer_ptr r)
  {
    return expect_object<obj::ratio>(l / obj::ratio_data(r->data, 1LL));
  }

  native_real operator/(obj::real_ptr l, obj::ratio_data const r)
  {
    return l->data / r.to_real();
  }

  native_real operator/(obj::ratio_data const l, obj::real_ptr r)
  {
    return l.to_real() / r->data;
  }

  native_real operator/(obj::ratio_data const l, native_real r)
  {
    return l.to_real() / r;
  }

  native_real operator/(native_real l, obj::ratio_data const r)
  {
    return l / r.to_real();
  }

  obj::ratio_ptr operator/(obj::ratio_data const l, native_integer r)
  {
    return l / make_box<obj::integer>(r);
  }

  object_ptr operator/(native_integer l, obj::ratio_data const r)
  {
    return obj::ratio_data(l, 1LL) / r;
  }

  native_bool operator==(obj::ratio_data const l, obj::ratio_data const r)
  {
    return l.numerator == r.numerator && l.denominator == r.denominator;
  }

  native_bool operator==(obj::integer_ptr l, obj::ratio_data const r)
  {
    return l->data * r.denominator == r.numerator;
  }

  native_bool operator==(obj::ratio_data const l, obj::integer_ptr r)
  {
    return l.numerator == r->data * l.denominator;
  }

  native_bool operator==(obj::real_ptr l, obj::ratio_data const r)
  {
    return std::fabs(l->data - r) < EPSILON;
  }

  native_bool operator==(obj::ratio_data const l, obj::real_ptr r)
  {
    return r == l;
  }

  native_bool operator==(obj::ratio_data const l, native_real r)
  {
    return std::fabs(l - r) < EPSILON;
  }

  native_bool operator==(native_real l, obj::ratio_data const r)
  {
    return r == l;
  }

  native_bool operator==(obj::ratio_data const l, native_integer r)
  {
    return l.numerator == r * l.denominator;
  }

  native_bool operator==(native_integer l, obj::ratio_data const r)
  {
    return l * r.denominator == r.numerator;
  }

  native_bool operator<(obj::ratio_data const l, obj::ratio_data const r)
  {
    return l.numerator * r.denominator < r.numerator * l.denominator;
  }

  native_bool operator<=(obj::ratio_data const l, obj::ratio_data const r)
  {
    return l.numerator * r.denominator <= r.numerator * l.denominator;
  }

  native_bool operator<(obj::integer_ptr l, obj::ratio_data const r)
  {
    return l->data * r.denominator < r.numerator;
  }

  native_bool operator<(obj::ratio_data const l, obj::integer_ptr r)
  {
    return l.numerator < r->data * l.denominator;
  }

  native_bool operator<=(obj::integer_ptr l, obj::ratio_data const r)
  {
    return l->data * r.denominator <= r.numerator;
  }

  native_bool operator<=(obj::ratio_data const l, obj::integer_ptr r)
  {
    return l.numerator <= r->data * l.denominator;
  }

  native_bool operator<(obj::real_ptr l, obj::ratio_data const r)
  {
    return l->data < r.to_real();
  }

  native_bool operator<(obj::ratio_data const l, obj::real_ptr r)
  {
    return l.to_real() < r->data;
  }

  native_bool operator<=(obj::real_ptr l, obj::ratio_data const r)
  {
    return l->data <= r.to_real();
  }

  native_bool operator<=(obj::ratio_data const l, obj::real_ptr r)
  {
    return l.to_real() <= r->data;
  }

  native_bool operator<(obj::ratio_data const l, native_real r)
  {
    return l.to_real() < r;
  }

  native_bool operator<(native_real l, obj::ratio_data const r)
  {
    return l < r.to_real();
  }

  native_bool operator<=(obj::ratio_data const l, native_real r)
  {
    return l.to_real() <= r;
  }

  native_bool operator<=(native_real l, obj::ratio_data const r)
  {
    return l <= r.to_real();
  }

  native_bool operator<(obj::ratio_data const l, native_integer r)
  {
    return l.numerator < r * l.denominator;
  }

  native_bool operator<(native_integer l, obj::ratio_data const r)
  {
    return l * r.denominator < r.numerator;
  }

  native_bool operator<=(obj::ratio_data const l, native_integer r)
  {
    return l.numerator <= r * l.denominator;
  }

  native_bool operator<=(native_integer l, obj::ratio_data const r)
  {
    return l * r.denominator <= r.numerator;
  }

  native_bool operator>(obj::ratio_data const l, obj::ratio_data const r)
  {
    return l.numerator * r.denominator > r.numerator * l.denominator;
  }

  native_bool operator>(obj::integer_ptr l, obj::ratio_data const r)
  {
    return l->data * r.denominator > r.numerator;
  }

  native_bool operator>(obj::ratio_data const l, obj::integer_ptr r)
  {
    return l.numerator > r->data * l.denominator;
  }

  native_bool operator>(obj::real_ptr l, obj::ratio_data const r)
  {
    return l->data > r.to_real();
  }

  native_bool operator>(obj::ratio_data const l, obj::real_ptr r)
  {
    return l.to_real() > r->data;
  }

  native_bool operator>(obj::ratio_data const l, native_real r)
  {
    return l.to_real() > r;
  }

  native_bool operator>(native_real l, obj::ratio_data const r)
  {
    return l > r.to_real();
  }

  native_bool operator>(obj::ratio_data const l, native_integer r)
  {
    return l.numerator > r * l.denominator;
  }

  native_bool operator>(native_integer l, obj::ratio_data const r)
  {
    return l * r.denominator > r.numerator;
  }

  native_bool operator>=(obj::ratio_data const l, obj::ratio_data const r)
  {
    return l.numerator * r.denominator >= r.numerator * l.denominator;
  }

  native_bool operator>=(obj::integer_ptr l, obj::ratio_data const r)
  {
    return l->data * r.denominator >= r.numerator;
  }

  native_bool operator>=(obj::ratio_data const l, obj::integer_ptr r)
  {
    return l.numerator >= r->data * l.denominator;
  }

  native_bool operator>=(obj::real_ptr l, obj::ratio_data const r)
  {
    return l->data >= r.to_real();
  }

  native_bool operator>=(obj::ratio_data const l, obj::real_ptr r)
  {
    return l.to_real() >= r->data;
  }

  native_bool operator>=(obj::ratio_data const l, native_real r)
  {
    return l.to_real() >= r;
  }

  native_bool operator>=(native_real l, obj::ratio_data const r)
  {
    return l >= r.to_real();
  }

  native_bool operator>=(obj::ratio_data const l, native_integer r)
  {
    return l.numerator >= r * l.denominator;
  }

  native_bool operator>=(native_integer l, obj::ratio_data const r)
  {
    return l * r.denominator >= r.numerator;
  }

  native_bool operator>(native_bool l, obj::ratio_data const r)
  {
    return (l ? 1LL : 0LL) > r;
  }

  native_bool operator<(native_bool l, obj::ratio_data const r)
  {
    return (l ? 1LL : 0LL) < r;
  }

  native_bool operator>(obj::ratio_data const l, native_bool r)
  {
    return l > (r ? 1LL : 0LL);
  }

  native_bool operator<(obj::ratio_data const l, native_bool r)
  {
    return l < (r ? 1LL : 0LL);
  }
}
