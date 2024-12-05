#include <jank/runtime/obj/ratio.hpp>
#include <fmt/compile.h>
#include <limits>

namespace jank::runtime
{

  static constexpr auto epsilon{ std::numeric_limits<native_real>::epsilon() };

  obj::ratio_data::ratio_data(native_integer const numerator, native_integer const denominator)
    : numerator{ numerator }
    , denominator{ denominator }
  {
    if(denominator == 0)
    {
      throw std::invalid_argument("Ratio denominator cannot be zero.");
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

  obj::ratio_data::ratio_data(obj::ratio_data const &data)
    : numerator{ data.numerator }
    , denominator{ data.denominator }
  {
  }

  obj::ratio::static_object(obj::ratio_data const &data)
    : data{ data }
  {
  }

  object_ptr obj::ratio::create(native_integer const numerator, native_integer const denominator)
  {
    obj::ratio_data data{ numerator, denominator };
    if(data.denominator == 1)
    {
      return make_box<obj::integer>(data.numerator);
    }
    return make_box<obj::ratio>(data);
  }

  native_real obj::ratio_data::to_real() const
  {
    return static_cast<native_real>(numerator) / denominator;
  }

  native_integer obj::ratio_data::to_integer() const
  {
    return numerator / denominator;
  }

  native_real obj::ratio::to_real() const
  {
    return data.to_real();
  }

  native_integer obj::ratio::to_integer() const
  {
    return data.to_integer();
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
      return data == expect_object<obj::integer>(&o)->data;
    }

    if(o.type == object_type::real)
    {
      return data == expect_object<obj::real>(&o)->data;
    }

    if(o.type == object_type::ratio)
    {
      return data == expect_object<obj::ratio>(&o)->data;
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

  namespace obj
  {
    object_ptr operator+(obj::ratio_data const &l, obj::ratio_data const &r)
    {
      auto const denom{ l.denominator * r.denominator };
      auto const num{ l.numerator * r.denominator + r.numerator * l.denominator };
      return obj::ratio::create(num, denom);
    }

    obj::ratio_ptr operator+(obj::integer_ptr const l, obj::ratio_data const &r)
    {
      return l->data + r;
    }

    obj::ratio_ptr operator+(obj::ratio_data const &l, obj::integer_ptr const r)
    {
      return r + l;
    }

    native_real operator+(obj::real_ptr const l, obj::ratio_data const &r)
    {
      return l->data + r.to_real();
    }

    native_real operator+(obj::ratio_data const &l, obj::real_ptr const r)
    {
      return l.to_real() + r->data;
    }

    native_real operator+(obj::ratio_data const &l, native_real const r)
    {
      return l.to_real() + r;
    }

    native_real operator+(native_real const l, obj::ratio_data const &r)
    {
      return l + r.to_real();
    }

    obj::ratio_ptr operator+(obj::ratio_data const &l, native_integer const r)
    {
      return make_box<obj::ratio>(obj::ratio_data(l.numerator + r * l.denominator, l.denominator));
    }

    obj::ratio_ptr operator+(native_integer const l, obj::ratio_data const &r)
    {
      return r + l;
    }

    object_ptr operator-(obj::ratio_data const &l, obj::ratio_data const &r)
    {
      auto const denom{ l.denominator * r.denominator };
      auto const num{ l.numerator * r.denominator - r.numerator * l.denominator };
      return obj::ratio::create(num, denom);
    }

    obj::ratio_ptr operator-(obj::integer_ptr const l, obj::ratio_data const &r)
    {
      return l->data - r;
    }

    obj::ratio_ptr operator-(obj::ratio_data const &l, obj::integer_ptr const r)
    {
      return l - r->data;
    }

    native_real operator-(obj::real_ptr const l, obj::ratio_data const &r)
    {
      return l->data - r.to_real();
    }

    native_real operator-(obj::ratio_data const &l, obj::real_ptr const r)
    {
      return l.to_real() - r->data;
    }

    native_real operator-(obj::ratio_data const &l, native_real const r)
    {
      return l.to_real() - r;
    }

    native_real operator-(native_real const l, obj::ratio_data const &r)
    {
      return l - r.to_real();
    }

    obj::ratio_ptr operator-(obj::ratio_data const &l, native_integer const r)
    {
      return make_box<obj::ratio>(obj::ratio_data(l.numerator - r * l.denominator, l.denominator));
    }

    obj::ratio_ptr operator-(native_integer const l, obj::ratio_data const &r)
    {
      return make_box<obj::ratio>(
        obj::ratio_data(l * r.denominator - r.numerator, r.denominator));
    }

    object_ptr operator*(obj::ratio_data const &l, obj::ratio_data const &r)
    {
      return obj::ratio::create(l.numerator * r.numerator, l.denominator * r.denominator);
    }

    object_ptr operator*(obj::integer_ptr const l, obj::ratio_data const &r)
    {
      return obj::ratio_data(l->data, 1LL) * r;
    }

    object_ptr operator*(obj::ratio_data const &l, obj::integer_ptr const r)
    {
      return l * obj::ratio_data(r->data, 1LL);
    }

    native_real operator*(obj::real_ptr const l, obj::ratio_data const &r)
    {
      return l->data * r.to_real();
    }

    native_real operator*(obj::ratio_data const &l, obj::real_ptr const r)
    {
      return l.to_real() * r->data;
    }

    native_real operator*(obj::ratio_data const &l, native_real const r)
    {
      return l.to_real() * r;
    }

    native_real operator*(native_real const l, obj::ratio_data const &r)
    {
      return l * r.to_real();
    }

    object_ptr operator*(obj::ratio_data const &l, native_integer const r)
    {
      return l * obj::ratio_data(r, 1LL);
    }

    object_ptr operator*(native_integer const l, obj::ratio_data const &r)
    {
      return r * l;
    }

    object_ptr operator/(obj::ratio_data const &l, obj::ratio_data const &r)
    {
      return obj::ratio::create(l.numerator * r.denominator, l.denominator * r.numerator);
    }

    object_ptr operator/(obj::integer_ptr const l, obj::ratio_data const &r)
    {
      return obj::ratio_data(l->data, 1LL) / r;
    }

    obj::ratio_ptr operator/(obj::ratio_data const &l, obj::integer_ptr const r)
    {
      return l / r->data;
    }

    native_real operator/(obj::real_ptr const l, obj::ratio_data const &r)
    {
      return l->data / r.to_real();
    }

    native_real operator/(obj::ratio_data const &l, obj::real_ptr const r)
    {
      return l.to_real() / r->data;
    }

    native_real operator/(obj::ratio_data const &l, native_real const r)
    {
      return l.to_real() / r;
    }

    native_real operator/(native_real const l, obj::ratio_data const &r)
    {
      return l / r.to_real();
    }

    obj::ratio_ptr operator/(obj::ratio_data const &l, native_integer const r)
    {
      return make_box<obj::ratio>(obj::ratio_data(l.numerator, l.denominator * r));
    }

    object_ptr operator/(native_integer const l, obj::ratio_data const &r)
    {
      return obj::ratio_data(l, 1LL) / r;
    }

    native_bool operator==(obj::ratio_data const &l, obj::ratio_data const &r)
    {
      return l.numerator == r.numerator && l.denominator == r.denominator;
    }

    native_bool operator==(obj::integer_ptr const l, obj::ratio_data const &r)
    {
      return l->data * r.denominator == r.numerator;
    }

    native_bool operator==(obj::ratio_data const &l, obj::integer_ptr const r)
    {
      return l.numerator == r->data * l.denominator;
    }

    native_bool operator==(obj::real_ptr const l, obj::ratio_data const &r)
    {
      return std::fabs(l->data - r) < epsilon;
    }

    native_bool operator==(obj::ratio_data const &l, obj::real_ptr const r)
    {
      return r == l;
    }

    native_bool operator==(obj::ratio_data const &l, native_real const r)
    {
      return std::fabs(l - r) < epsilon;
    }

    native_bool operator==(native_real const l, obj::ratio_data const &r)
    {
      return r == l;
    }

    native_bool operator==(obj::ratio_data const &l, native_integer const r)
    {
      return l.numerator == r * l.denominator;
    }

    native_bool operator==(native_integer const l, obj::ratio_data const &r)
    {
      return l * r.denominator == r.numerator;
    }

    native_bool operator<(obj::ratio_data const &l, obj::ratio_data const &r)
    {
      return l.numerator * r.denominator < r.numerator * l.denominator;
    }

    native_bool operator<=(obj::ratio_data const &l, obj::ratio_data const &r)
    {
      return l.numerator * r.denominator <= r.numerator * l.denominator;
    }

    native_bool operator<(obj::integer_ptr const l, obj::ratio_data const &r)
    {
      return l->data * r.denominator < r.numerator;
    }

    native_bool operator<(obj::ratio_data const &l, obj::integer_ptr const r)
    {
      return l.numerator < r->data * l.denominator;
    }

    native_bool operator<=(obj::integer_ptr const l, obj::ratio_data const &r)
    {
      return l->data * r.denominator <= r.numerator;
    }

    native_bool operator<=(obj::ratio_data const &l, obj::integer_ptr const r)
    {
      return l.numerator <= r->data * l.denominator;
    }

    native_bool operator<(obj::real_ptr const l, obj::ratio_data const &r)
    {
      return l->data < r.to_real();
    }

    native_bool operator<(obj::ratio_data const &l, obj::real_ptr const r)
    {
      return l.to_real() < r->data;
    }

    native_bool operator<=(obj::real_ptr const l, obj::ratio_data const &r)
    {
      return l->data <= r.to_real();
    }

    native_bool operator<=(obj::ratio_data const &l, obj::real_ptr const r)
    {
      return l.to_real() <= r->data;
    }

    native_bool operator<(obj::ratio_data const &l, native_real const r)
    {
      return l.to_real() < r;
    }

    native_bool operator<(native_real const l, obj::ratio_data const &r)
    {
      return l < r.to_real();
    }

    native_bool operator<=(obj::ratio_data const &l, native_real const r)
    {
      return l.to_real() <= r;
    }

    native_bool operator<=(native_real const l, obj::ratio_data const &r)
    {
      return l <= r.to_real();
    }

    native_bool operator<(obj::ratio_data const &l, native_integer const r)
    {
      return l.numerator < r * l.denominator;
    }

    native_bool operator<(native_integer const l, obj::ratio_data const &r)
    {
      return l * r.denominator < r.numerator;
    }

    native_bool operator<=(obj::ratio_data const &l, native_integer const r)
    {
      return l.numerator <= r * l.denominator;
    }

    native_bool operator<=(native_integer const l, obj::ratio_data const &r)
    {
      return l * r.denominator <= r.numerator;
    }

    native_bool operator>(obj::ratio_data const &l, obj::ratio_data const &r)
    {
      return l.numerator * r.denominator > r.numerator * l.denominator;
    }

    native_bool operator>(obj::integer_ptr const l, obj::ratio_data const &r)
    {
      return l->data * r.denominator > r.numerator;
    }

    native_bool operator>(obj::ratio_data const &l, obj::integer_ptr const r)
    {
      return l.numerator > r->data * l.denominator;
    }

    native_bool operator>(obj::real_ptr const l, obj::ratio_data const &r)
    {
      return l->data > r.to_real();
    }

    native_bool operator>(obj::ratio_data const &l, obj::real_ptr const r)
    {
      return l.to_real() > r->data;
    }

    native_bool operator>(obj::ratio_data const &l, native_real const r)
    {
      return l.to_real() > r;
    }

    native_bool operator>(native_real const l, obj::ratio_data const &r)
    {
      return l > r.to_real();
    }

    native_bool operator>(obj::ratio_data const &l, native_integer const r)
    {
      return l.numerator > r * l.denominator;
    }

    native_bool operator>(native_integer const l, obj::ratio_data const &r)
    {
      return l * r.denominator > r.numerator;
    }

    native_bool operator>=(obj::ratio_data const &l, obj::ratio_data const &r)
    {
      return l.numerator * r.denominator >= r.numerator * l.denominator;
    }

    native_bool operator>=(obj::integer_ptr const l, obj::ratio_data const &r)
    {
      return l->data * r.denominator >= r.numerator;
    }

    native_bool operator>=(obj::ratio_data const &l, obj::integer_ptr const r)
    {
      return l.numerator >= r->data * l.denominator;
    }

    native_bool operator>=(obj::real_ptr const l, obj::ratio_data const &r)
    {
      return l->data >= r.to_real();
    }

    native_bool operator>=(obj::ratio_data const &l, obj::real_ptr const r)
    {
      return l.to_real() >= r->data;
    }

    native_bool operator>=(obj::ratio_data const &l, native_real const r)
    {
      return l.to_real() >= r;
    }

    native_bool operator>=(native_real const l, obj::ratio_data const &r)
    {
      return l >= r.to_real();
    }

    native_bool operator>=(obj::ratio_data const &l, native_integer const r)
    {
      return l.numerator >= r * l.denominator;
    }

    native_bool operator>=(native_integer const l, obj::ratio_data const &r)
    {
      return l * r.denominator >= r.numerator;
    }

    native_bool operator>(native_bool l, obj::ratio_data const &r)
    {
      return (l ? 1LL : 0LL) > r;
    }

    native_bool operator<(native_bool l, obj::ratio_data const &r)
    {
      return (l ? 1LL : 0LL) < r;
    }

    native_bool operator>(obj::ratio_data const &l, native_bool r)
    {
      return l > (r ? 1LL : 0LL);
    }

    native_bool operator<(obj::ratio_data const &l, native_bool r)
    {
      return l < (r ? 1LL : 0LL);
    }
  }
}
