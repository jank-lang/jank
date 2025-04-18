#include <limits>
#include <numeric>
#include <cmath>

#include <jank/runtime/obj/ratio.hpp>
#include <jank/runtime/visit.hpp>

namespace jank::runtime::obj
{
  static constexpr auto epsilon{ std::numeric_limits<f64>::epsilon() };

  ratio_data::ratio_data(i64 const numerator, i64 const denominator)
    : numerator{ numerator }
    , denominator{ denominator }
  {
    if(denominator == 0)
    {
      throw std::invalid_argument{ "Ratio denominator cannot be zero." };
    }
    auto const gcd{ std::gcd(numerator, denominator) };
    this->numerator /= gcd;
    this->denominator /= gcd;

    if(denominator < 0)
    {
      this->numerator = -1 * this->numerator;
      this->denominator = -1 * this->denominator;
    }
  }

  ratio::ratio(ratio_data const &data)
    : data{ data }
  {
  }

  object_ref ratio::create(i64 const numerator, i64 const denominator)
  {
    ratio_data const data{ numerator, denominator };
    if(data.denominator == 1)
    {
      return make_box<integer>(data.numerator);
    }
    return make_box<ratio>(data);
  }

  f64 ratio_data::to_real() const
  {
    return static_cast<f64>(numerator) / static_cast<f64>(denominator);
  }

  i64 ratio_data::to_integer() const
  {
    return numerator / denominator;
  }

  f64 ratio::to_real() const
  {
    return data.to_real();
  }

  i64 ratio::to_integer() const
  {
    return data.to_integer();
  }

  void ratio::to_string(util::string_builder &buff) const
  {
    buff(data.numerator)('/')(data.denominator);
  }

  jtl::immutable_string ratio::to_string() const
  {
    util::string_builder buff;
    to_string(buff);
    return buff.release();
  }

  jtl::immutable_string ratio::to_code_string() const
  {
    return to_string();
  }

  native_hash ratio::to_hash() const
  {
    return hash::combine(hash::integer(data.numerator), hash::integer(data.denominator));
  }

  bool ratio::equal(object const &o) const
  {
    if(o.type == object_type::integer)
    {
      return data == expect_object<integer>(&o)->data;
    }

    if(o.type == object_type::real)
    {
      return data == expect_object<real>(&o)->data;
    }

    if(o.type == object_type::ratio)
    {
      return data == expect_object<ratio>(&o)->data;
    }

    return false;
  }

  i64 ratio::compare(object const &o) const
  {
    return visit_number_like(
      [this](auto const typed_o) -> i64 { return (data > typed_o->data) - (data < typed_o->data); },
      &o);
  }

  i64 ratio::compare(ratio const &o) const
  {
    return (data > o.data) - (data < o.data);
  }

  object_ref operator+(ratio_data const &l, ratio_data const &r)
  {
    auto const denom{ l.denominator * r.denominator };
    auto const num{ (l.numerator * r.denominator) + (r.numerator * l.denominator) };
    return ratio::create(num, denom);
  }

  ratio_ref operator+(integer_ref const l, ratio_data const &r)
  {
    return l->data + r;
  }

  ratio_ref operator+(ratio_data const &l, integer_ref const r)
  {
    return r + l;
  }

  f64 operator+(real_ref const l, ratio_data const &r)
  {
    return l->data + r.to_real();
  }

  f64 operator+(ratio_data const &l, real_ref const r)
  {
    return l.to_real() + r->data;
  }

  f64 operator+(ratio_data const &l, f64 const r)
  {
    return l.to_real() + r;
  }

  f64 operator+(f64 const l, ratio_data const &r)
  {
    return l + r.to_real();
  }

  ratio_ref operator+(ratio_data const &l, i64 const r)
  {
    return make_box<ratio>(ratio_data(l.numerator + (r * l.denominator), l.denominator));
  }

  ratio_ref operator+(i64 const l, ratio_data const &r)
  {
    return r + l;
  }

  object_ref operator-(ratio_data const &l, ratio_data const &r)
  {
    auto const denom{ l.denominator * r.denominator };
    auto const num{ (l.numerator * r.denominator) - (r.numerator * l.denominator) };
    return ratio::create(num, denom);
  }

  ratio_ref operator-(integer_ref const l, ratio_data const &r)
  {
    return l->data - r;
  }

  ratio_ref operator-(ratio_data const &l, integer_ref const r)
  {
    return l - r->data;
  }

  f64 operator-(real_ref const l, ratio_data const &r)
  {
    return l->data - r.to_real();
  }

  f64 operator-(ratio_data const &l, real_ref const r)
  {
    return l.to_real() - r->data;
  }

  f64 operator-(ratio_data const &l, f64 const r)
  {
    return l.to_real() - r;
  }

  f64 operator-(f64 const l, ratio_data const &r)
  {
    return l - r.to_real();
  }

  ratio_ref operator-(ratio_data const &l, i64 const r)
  {
    return make_box<ratio>(ratio_data(l.numerator - (r * l.denominator), l.denominator));
  }

  ratio_ref operator-(i64 const l, ratio_data const &r)
  {
    return make_box<ratio>(ratio_data((l * r.denominator) - r.numerator, r.denominator));
  }

  object_ref operator*(ratio_data const &l, ratio_data const &r)
  {
    return ratio::create(l.numerator * r.numerator, l.denominator * r.denominator);
  }

  object_ref operator*(integer_ref const l, ratio_data const &r)
  {
    return ratio_data(l->data, 1ll) * r;
  }

  object_ref operator*(ratio_data const &l, integer_ref const r)
  {
    return l * ratio_data(r->data, 1ll);
  }

  f64 operator*(real_ref const l, ratio_data const &r)
  {
    return l->data * r.to_real();
  }

  f64 operator*(ratio_data const &l, real_ref const r)
  {
    return l.to_real() * r->data;
  }

  f64 operator*(ratio_data const &l, f64 const r)
  {
    return l.to_real() * r;
  }

  f64 operator*(f64 const l, ratio_data const &r)
  {
    return l * r.to_real();
  }

  object_ref operator*(ratio_data const &l, i64 const r)
  {
    return l * ratio_data(r, 1ll);
  }

  object_ref operator*(i64 const l, ratio_data const &r)
  {
    return r * l;
  }

  object_ref operator/(ratio_data const &l, ratio_data const &r)
  {
    return ratio::create(l.numerator * r.denominator, l.denominator * r.numerator);
  }

  object_ref operator/(integer_ref const l, ratio_data const &r)
  {
    return ratio_data(l->data, 1ll) / r;
  }

  ratio_ref operator/(ratio_data const &l, integer_ref const r)
  {
    return l / r->data;
  }

  f64 operator/(real_ref const l, ratio_data const &r)
  {
    return l->data / r.to_real();
  }

  f64 operator/(ratio_data const &l, real_ref const r)
  {
    return l.to_real() / r->data;
  }

  f64 operator/(ratio_data const &l, f64 const r)
  {
    return l.to_real() / r;
  }

  f64 operator/(f64 const l, ratio_data const &r)
  {
    return l / r.to_real();
  }

  ratio_ref operator/(ratio_data const &l, i64 const r)
  {
    return make_box<ratio>(ratio_data(l.numerator, l.denominator * r));
  }

  object_ref operator/(i64 const l, ratio_data const &r)
  {
    return ratio_data(l, 1ll) / r;
  }

  bool operator==(ratio_data const &l, ratio_data const &r)
  {
    return l.numerator == r.numerator && l.denominator == r.denominator;
  }

  bool operator==(integer_ref const l, ratio_data const &r)
  {
    return l->data * r.denominator == r.numerator;
  }

  bool operator==(ratio_data const &l, integer_ref const r)
  {
    return l.numerator == r->data * l.denominator;
  }

  bool operator==(real_ref const l, ratio_data const &r)
  {
    return std::fabs(l->data - r) < epsilon;
  }

  bool operator==(ratio_data const &l, real_ref const r)
  {
    return r == l;
  }

  bool operator==(ratio_data const &l, f64 const r)
  {
    return std::fabs(l - r) < epsilon;
  }

  bool operator==(f64 const l, ratio_data const &r)
  {
    return r == l;
  }

  bool operator==(ratio_data const &l, i64 const r)
  {
    return l.numerator == r * l.denominator;
  }

  bool operator==(i64 const l, ratio_data const &r)
  {
    return l * r.denominator == r.numerator;
  }

  bool operator<(ratio_data const &l, ratio_data const &r)
  {
    return l.numerator * r.denominator < r.numerator * l.denominator;
  }

  bool operator<=(ratio_data const &l, ratio_data const &r)
  {
    return l.numerator * r.denominator <= r.numerator * l.denominator;
  }

  bool operator<(integer_ref const l, ratio_data const &r)
  {
    return l->data * r.denominator < r.numerator;
  }

  bool operator<(ratio_data const &l, integer_ref const r)
  {
    return l.numerator < r->data * l.denominator;
  }

  bool operator<=(integer_ref const l, ratio_data const &r)
  {
    return l->data * r.denominator <= r.numerator;
  }

  bool operator<=(ratio_data const &l, integer_ref const r)
  {
    return l.numerator <= r->data * l.denominator;
  }

  bool operator<(real_ref const l, ratio_data const &r)
  {
    return l->data < r.to_real();
  }

  bool operator<(ratio_data const &l, real_ref const r)
  {
    return l.to_real() < r->data;
  }

  bool operator<=(real_ref const l, ratio_data const &r)
  {
    return l->data <= r.to_real();
  }

  bool operator<=(ratio_data const &l, real_ref const r)
  {
    return l.to_real() <= r->data;
  }

  bool operator<(ratio_data const &l, f64 const r)
  {
    return l.to_real() < r;
  }

  bool operator<(f64 const l, ratio_data const &r)
  {
    return l < r.to_real();
  }

  bool operator<=(ratio_data const &l, f64 const r)
  {
    return l.to_real() <= r;
  }

  bool operator<=(f64 const l, ratio_data const &r)
  {
    return l <= r.to_real();
  }

  bool operator<(ratio_data const &l, i64 const r)
  {
    return l.numerator < r * l.denominator;
  }

  bool operator<(i64 const l, ratio_data const &r)
  {
    return l * r.denominator < r.numerator;
  }

  bool operator<=(ratio_data const &l, i64 const r)
  {
    return l.numerator <= r * l.denominator;
  }

  bool operator<=(i64 const l, ratio_data const &r)
  {
    return l * r.denominator <= r.numerator;
  }

  bool operator>(ratio_data const &l, ratio_data const &r)
  {
    return l.numerator * r.denominator > r.numerator * l.denominator;
  }

  bool operator>(integer_ref const l, ratio_data const &r)
  {
    return l->data * r.denominator > r.numerator;
  }

  bool operator>(ratio_data const &l, integer_ref const r)
  {
    return l.numerator > r->data * l.denominator;
  }

  bool operator>(real_ref const l, ratio_data const &r)
  {
    return l->data > r.to_real();
  }

  bool operator>(ratio_data const &l, real_ref const r)
  {
    return l.to_real() > r->data;
  }

  bool operator>(ratio_data const &l, f64 const r)
  {
    return l.to_real() > r;
  }

  bool operator>(f64 const l, ratio_data const &r)
  {
    return l > r.to_real();
  }

  bool operator>(ratio_data const &l, i64 const r)
  {
    return l.numerator > r * l.denominator;
  }

  bool operator>(i64 const l, ratio_data const &r)
  {
    return l * r.denominator > r.numerator;
  }

  bool operator>=(ratio_data const &l, ratio_data const &r)
  {
    return l.numerator * r.denominator >= r.numerator * l.denominator;
  }

  bool operator>=(integer_ref const l, ratio_data const &r)
  {
    return l->data * r.denominator >= r.numerator;
  }

  bool operator>=(ratio_data const &l, integer_ref const r)
  {
    return l.numerator >= r->data * l.denominator;
  }

  bool operator>=(real_ref const l, ratio_data const &r)
  {
    return l->data >= r.to_real();
  }

  bool operator>=(ratio_data const &l, real_ref const r)
  {
    return l.to_real() >= r->data;
  }

  bool operator>=(ratio_data const &l, f64 const r)
  {
    return l.to_real() >= r;
  }

  bool operator>=(f64 const l, ratio_data const &r)
  {
    return l >= r.to_real();
  }

  bool operator>=(ratio_data const &l, i64 const r)
  {
    return l.numerator >= r * l.denominator;
  }

  bool operator>=(i64 const l, ratio_data const &r)
  {
    return l * r.denominator >= r.numerator;
  }

  bool operator>(bool l, ratio_data const &r)
  {
    return (l ? 1ll : 0ll) > r;
  }

  bool operator<(bool l, ratio_data const &r)
  {
    return (l ? 1ll : 0ll) < r;
  }

  bool operator>(ratio_data const &l, bool const r)
  {
    return l > (r ? 1ll : 0ll);
  }

  bool operator<(ratio_data const &l, bool const r)
  {
    return l < (r ? 1ll : 0ll);
  }
}
