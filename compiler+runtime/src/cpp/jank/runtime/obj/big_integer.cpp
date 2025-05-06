#include <boost/multiprecision/cpp_int.hpp>

#include <jank/runtime/obj/big_integer.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/obj/ratio.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime
{

  native_real operator+(native_big_integer const &l, native_real const &r)
  {
    return obj::big_integer::to_native_real(l) + r;
  }

  native_real operator+(native_real const &l, native_big_integer const &r)
  {
    return l + obj::big_integer::to_native_real(r);
  }

  native_real operator-(native_big_integer const &l, native_real const &r)
  {
    return obj::big_integer::to_native_real(l) - r;
  }

  native_real operator-(native_real const &l, native_big_integer const &r)
  {
    return l - obj::big_integer::to_native_real(r);
  }

  native_real operator*(native_big_integer const &l, native_real const &r)
  {
    return obj::big_integer::to_native_real(l) * r;
  }

  native_real operator*(native_real const &l, native_big_integer const &r)
  {
    return r * l;
  }

  native_real operator/(native_big_integer const &l, native_real const &r)
  {
    return obj::big_integer::to_native_real(l) / r;
  }

  native_real operator/(native_real const &l, native_big_integer const &r)
  {
    return l / obj::big_integer::to_native_real(r);
  }

  native_bool operator>(native_big_integer const &l, native_real const &r)
  {
    return obj::big_integer::to_native_real(l) > r;
  }

  native_bool operator>(native_real const &l, native_big_integer const &r)
  {
    return l > obj::big_integer::to_native_real(r);
  }

  native_bool operator==(native_big_integer const &l, native_real const &r)
  {
    constexpr native_real epsilon = std::numeric_limits<native_real>::epsilon();
    return std::abs(l - r) < epsilon;
  }

  native_bool operator==(native_real const &l, native_big_integer const &r)
  {
    return r == l;
  }

  native_bool operator!=(native_big_integer const &l, native_real const &r)
  {
    return !(l == r);
  }

  native_bool operator!=(native_real const &l, native_big_integer const &r)
  {
    return r != l; // Reuse
  }

  native_bool operator<(native_big_integer const &l, native_real const &r)
  {
    return obj::big_integer::to_native_real(l) < r;
  }

  native_bool operator<(native_real const &l, native_big_integer const &r)
  {
    return l < obj::big_integer::to_native_real(r);
  }

  native_bool operator<=(native_big_integer const &l, native_real const &r)
  {
    return l < r || l == r;
  }

  native_bool operator<=(native_real const &l, native_big_integer const &r)
  {
    return l < r || l == r;
  }

  native_bool operator>=(native_big_integer const &l, native_real const &r)
  {
    return l > r || l == r;
  }

  native_bool operator>=(native_real const &l, native_big_integer const &r)
  {
    return l > r || l == r;
  }

}

namespace jank::runtime::obj
{
  big_integer::big_integer()
    : data(0)
  {
  }

  big_integer::big_integer(native_big_integer const &val)
    : data(val)
  {
  }

  big_integer::big_integer(native_big_integer &&val)
    : data(std::move(val))
  {
  }

  big_integer::big_integer(native_integer val)
    : data(val)
  {
  }

  void big_integer::init(native_persistent_string_view const &s)
  {
    if(s.empty())
    {
      throw std::runtime_error(util::format("Failed to construct BigInteger from empty string"));
    }

    try
    {
      data.assign(std::string(s));
    }
    catch(std::exception const &e)
    {
      throw std::runtime_error(
        util::format("Failed to construct BigInteger from string '{}': {}", s, e.what()));
    }
  }

  big_integer::big_integer(native_persistent_string_view const &s)
  {
    init(s);
  }

  big_integer::big_integer(native_persistent_string_view const &s,
                           native_integer radix,
                           native_bool is_negative)
    : big_integer()
  {
    /* Radix passed from lexer and it's made sure to be between 2 and 36. */
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
      /* For all other radix, we need to manually parse the number. */
      native_big_integer temp_val = 0;
      native_big_integer current_radix_power = 1;
      for(auto it = s.rbegin(); it != s.rend(); ++it)
      {
        int digit_val;
        char c = std::tolower(*it);
        if(c >= '0' && c <= '9')
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

  object_ptr big_integer::create(native_persistent_string_view const &s,
                                 native_integer radix,
                                 native_bool is_negative)
  {
    return make_box<big_integer>(s, radix, is_negative);
  }

  native_bool big_integer::equal(object const &o) const
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
          < std::numeric_limits<native_real>::epsilon();
      }
      catch(...)
      {
        return false;
      }
    }

    return false;
  }

  native_persistent_string big_integer::to_string() const
  {
    return data.str();
  }

  void big_integer::to_string(util::string_builder &buff) const
  {
    buff(data.str());
  }

  native_persistent_string big_integer::to_code_string() const
  {
    return to_string();
  }

  template <class T>
  inline void hash_combine(std::size_t &seed, T const &v)
  {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }

  native_hash big_integer::to_hash(native_big_integer const &data)
  {
    auto const &backend = data.backend();

    auto const *limbs = backend.limbs();
    auto const size = backend.size();
    auto const sign = backend.sign();

    std::size_t seed = static_cast<std::size_t>(sign);
    for(unsigned i = 0; i < size; ++i)
    {
      hash_combine(seed, limbs[i]);
    }

    return static_cast<native_hash>(seed);
  }

  native_hash big_integer::to_hash() const
  {
    return big_integer::to_hash(data);
  }

  native_integer big_integer::compare(object const &o) const
  {
    return visit_number_like(
      [this](auto const typed_o) -> native_integer {
        /* Converts to native_integer for now. */
        return (data > typed_o->data) - (data < typed_o->data);
      },
      [&]() -> native_integer {
        throw std::runtime_error{ util::format("not comparable: {}", runtime::to_string(&o)) };
      },
      &o);
  }

  native_integer big_integer::compare(big_integer const &o) const
  {
    return data.compare(o.data);
  }

  native_big_integer big_integer::gcd(native_big_integer const &l, native_big_integer const &r)
  {
    return boost::multiprecision::gcd(l, r);
  }

  native_integer big_integer::to_native_integer(native_big_integer const &d)
  {
    if(d > std::numeric_limits<native_integer>::max()
       || d < std::numeric_limits<native_integer>::min())
    {
      throw std::runtime_error{ "Value out of range for integer." };
    }
    return static_cast<native_integer>(d);
  }

  native_integer big_integer::to_integer() const
  {
    return big_integer::to_native_integer(data);
  }

  native_real big_integer::to_native_real(native_big_integer const &data)
  {
    try
    {
      return data.template convert_to<native_real>();
    }
    catch(std::overflow_error const &)
    {
      return data < 0 ? -std::numeric_limits<native_real>::infinity()
                      : std::numeric_limits<native_real>::infinity();
    }
    catch(std::exception const &e)
    {
      throw std::runtime_error(
        util::format("Error converting BigInteger to native_real: {}", e.what()));
    }
  }

  native_real big_integer::to_real() const
  {
    return big_integer::to_native_real(data);
  }


} // namespace jank::runtime::obj
