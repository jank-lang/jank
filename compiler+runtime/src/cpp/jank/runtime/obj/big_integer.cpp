#include <jank/runtime/obj/big_integer.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/util/fmt.hpp>
#include <ranges>

namespace jank::runtime
{

  f64 operator+(native_big_integer const &l, f64 const &r)
  {
    return obj::big_integer::to_f64(l) + r;
  }

  f64 operator+(f64 const &l, native_big_integer const &r)
  {
    return l + obj::big_integer::to_f64(r);
  }

  f64 operator-(native_big_integer const &l, f64 const &r)
  {
    return obj::big_integer::to_f64(l) - r;
  }

  f64 operator-(f64 const &l, native_big_integer const &r)
  {
    return l - obj::big_integer::to_f64(r);
  }

  f64 operator*(native_big_integer const &l, f64 const &r)
  {
    return obj::big_integer::to_f64(l) * r;
  }

  f64 operator*(f64 const &l, native_big_integer const &r)
  {
    return r * l;
  }

  f64 operator/(native_big_integer const &l, f64 const &r)
  {
    return obj::big_integer::to_f64(l) / r;
  }

  f64 operator/(f64 const &l, native_big_integer const &r)
  {
    return l / obj::big_integer::to_f64(r);
  }

  bool operator>(native_big_integer const &l, f64 const &r)
  {
    return obj::big_integer::to_f64(l) > r;
  }

  bool operator>(f64 const &l, native_big_integer const &r)
  {
    return l > obj::big_integer::to_f64(r);
  }

  bool operator==(native_big_integer const &l, f64 const &r)
  {
    constexpr f64 epsilon{ std::numeric_limits<f64>::epsilon() };
    return std::abs(l - r) < epsilon;
  }

  bool operator==(f64 const &l, native_big_integer const &r)
  {
    return r == l;
  }

  bool operator!=(native_big_integer const &l, f64 const &r)
  {
    return !(l == r);
  }

  bool operator!=(f64 const &l, native_big_integer const &r)
  {
    return r != l;
  }

  bool operator<(native_big_integer const &l, f64 const &r)
  {
    return obj::big_integer::to_f64(l) < r;
  }

  bool operator<(f64 const &l, native_big_integer const &r)
  {
    return l < obj::big_integer::to_f64(r);
  }

  bool operator<=(native_big_integer const &l, f64 const &r)
  {
    return l < r || l == r;
  }

  bool operator<=(f64 const &l, native_big_integer const &r)
  {
    return l < r || l == r;
  }

  bool operator>=(native_big_integer const &l, f64 const &r)
  {
    return l > r || l == r;
  }

  bool operator>=(f64 const &l, native_big_integer const &r)
  {
    return l > r || l == r;
  }

}

namespace jank::runtime::obj
{
  big_integer::big_integer(native_big_integer const &val)
    : data(val)
  {
  }

  big_integer::big_integer(native_big_integer &&val)
    : data(std::move(val))
  {
  }

  big_integer::big_integer(i64 const val)
    : data(val)
  {
  }

  void big_integer::init(jtl::immutable_string const &s)
  {
    if(s.empty())
    {
      throw std::runtime_error(util::format("Failed to construct BigInteger from empty string"));
    }

    try
    {
      if(s.ends_with('N'))
      {
        data.assign(std::string(s.substr(0, s.size() - 1)));
      }
      else
      {
        data.assign(std::string(s));
      }
    }
    catch(std::exception const &e)
    {
      throw std::runtime_error(
        util::format("Failed to construct BigInteger from string '{}': {}", s, e.what()));
    }
  }

  big_integer::big_integer(jtl::immutable_string const &s)
  {
    init(s);
  }

  big_integer::big_integer(jtl::immutable_string const &s, i64 const radix, bool const is_negative)
  {
    /* Radix passed from lexer, and it's made sure to be between 2 and 36. */
    if(radix == 10)
    {
      init(s);
    }
    else if(radix == 8)
    {
      init("0" + std::string(s));
    }
    else if(radix == 16)
    {
      init("0x" + std::string(s));
    }
    else
    {
      /* For all other radixes, we need to manually parse the number. */
      native_big_integer temp_val{};
      native_big_integer current_radix_power{ 1 };
      for(char const it : std::ranges::reverse_view(s))
      {
        int digit_val{};
        if(int const c = std::tolower(it); c >= '0' && c <= '9')
        {
          digit_val = c - '0';
        }
        else
        {
          digit_val = c - 'a' + 10;
        }
        temp_val += current_radix_power * digit_val;
        current_radix_power *= radix;
      }
      data = temp_val;
    }

    if(is_negative)
    {
      data *= -1;
    }
  }

  object_ref
  big_integer::create(jtl::immutable_string const &s, i64 const radix, bool const is_negative)
  {
    return make_box<big_integer>(s, radix, is_negative);
  }

  bool big_integer::equal(object const &o) const
  {
    if(o.type == object_type::big_integer)
    {
      return data == expect_object<big_integer>(&o)->data;
    }
    if(o.type == object_type::integer)
    {
      return data == expect_object<integer>(&o)->data;
    }
    if(o.type == object_type::real)
    {
      try
      {
        return std::fabs(this->to_real() - expect_object<real>(&o)->data)
          < std::numeric_limits<f64>::epsilon();
      }
      catch(...)
      {
        return false;
      }
    }

    return false;
  }

  jtl::immutable_string big_integer::to_string() const
  {
    return data.str() + 'N';
  }

  void big_integer::to_string(util::string_builder &buff) const
  {
    buff(to_string());
  }

  jtl::immutable_string big_integer::to_code_string() const
  {
    return to_string();
  }

  template <class T>
  static void hash_combine(std::size_t &seed, T const &v)
  {
    std::hash<T> const hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }

  uhash big_integer::to_hash(native_big_integer const &data)
  {
    auto const &backend{ data.backend() };

    auto const *limbs{ backend.limbs() };
    auto const size{ backend.size() };
    auto const sign{ backend.sign() };

    auto seed{ static_cast<std::size_t>(sign) };
    for(unsigned i = 0; i < size; ++i)
    {
      hash_combine(seed, limbs[i]);
    }

    return static_cast<uhash>(seed);
  }

  uhash big_integer::to_hash() const
  {
    return big_integer::to_hash(data);
  }

  i64 big_integer::compare(object const &o) const
  {
    return visit_number_like(
      [this](auto const typed_o) -> i64 { return (data > typed_o->data) - (data < typed_o->data); },
      [&]() -> i64 {
        throw std::runtime_error{ util::format("not comparable: {}", runtime::to_string(&o)) };
      },
      &o);
  }

  i64 big_integer::compare(big_integer const &o) const
  {
    return data.compare(o.data);
  }

  native_big_integer big_integer::gcd(native_big_integer const &l, native_big_integer const &r)
  {
    /* NOLINTNEXTLINE(clang-analyzer-core.StackAddressEscape) */
    return boost::multiprecision::gcd(l, r);
  }

  i64 big_integer::to_i64(native_big_integer const &d)
  {
    if(d > std::numeric_limits<i64>::max() || d < std::numeric_limits<i64>::min())
    {
      throw std::runtime_error{ "Value out of range for integer." };
    }
    return static_cast<i64>(d);
  }

  i64 big_integer::to_integer() const
  {
    return big_integer::to_i64(data);
  }

  f64 big_integer::to_f64(native_big_integer const &data)
  {
    try
    {
      return data.convert_to<f64>();
    }
    catch(std::overflow_error const &)
    {
      return data < 0 ? -std::numeric_limits<f64>::infinity()
                      : std::numeric_limits<f64>::infinity();
    }
    catch(std::exception const &e)
    {
      throw std::runtime_error(util::format("Error converting BigInteger to f64: {}", e.what()));
    }
  }

  f64 big_integer::to_real() const
  {
    return big_integer::to_f64(data);
  }

}
