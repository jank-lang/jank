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
  native_bool operator>(native_big_integer const &l, native_real const &r)
  {
    return obj::big_integer::to_native_real(l) > r;
  }

  native_bool operator>(native_real const &l, native_big_integer const &r)
  {
    return l > obj::big_integer::to_native_real(r);
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

  big_integer::big_integer(native_persistent_string_view const &s)
  {
    try
    {
      data.assign(std::string(s));
    }
    catch(std::exception const &e)
    {
      // Consider re-throwing a Jank-specific exception or handling more gracefully
      throw std::runtime_error(
        util::format("Failed to construct BigInteger from string '{}': {}", s, e.what()));
    }
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
        return (to_integer() > typed_o->data) - (to_integer() < typed_o->data);
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
    // This implementation extracts the lower bits corresponding to a native_integer,
    // effectively performing arithmetic modulo 2^N (N=64 typically).

    using LimbType = boost::multiprecision::limb_type;
    constexpr std::size_t bits_per_limb = sizeof(LimbType) * CHAR_BIT;
    // Use unsigned long long for bit manipulation, then cast at the end
    constexpr std::size_t target_bits = sizeof(unsigned long long) * CHAR_BIT;

    auto const &backend = d.backend();
    auto const *limbs_ptr = backend.limbs();
    auto const num_limbs = backend.size();
    auto const sign = backend.sign();

    unsigned long long unsigned_result = 0;

    // Iterate through limbs contributing to the target bit width
    for(std::size_t i = 0;; ++i)
    {
      std::size_t current_limb_start_bit = i * bits_per_limb;
      if(current_limb_start_bit >= target_bits)
      {
        break; // Past target width
      }
      if(i >= num_limbs)
      {
        break; // Ran out of limbs
      }

      LimbType current_limb = limbs_ptr[i];

      // Mask if only part of the limb contributes
      std::size_t bits_to_take = bits_per_limb;
      if(current_limb_start_bit + bits_to_take > target_bits)
      {
        bits_to_take = target_bits - current_limb_start_bit;
        // Create mask: (1 << bits_to_take) - 1
        // Avoid shifting by full width or more
        if(bits_to_take < bits_per_limb)
        {
          LimbType mask = (static_cast<LimbType>(1) << bits_to_take) - 1;
          current_limb &= mask;
        } // else: no mask needed as we take the whole limb (up to target_bits)
      }

      // Combine into result
      unsigned_result |= (static_cast<unsigned long long>(current_limb) << current_limb_start_bit);
    }

    // Handle sign using two's complement logic for wrapping
    // Check the boolean sign variable directly
    if(sign) // If true, the original number was negative
    {
      return static_cast<native_integer>(~unsigned_result + 1);
    }
    else
    {
      return static_cast<long long>(unsigned_result);
    }
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

  native_real operator+(native_big_integer const &l, native_real const &r)
  {
    return big_integer::to_native_real(l) + r;
  }

  native_real operator+(native_real const &l, native_big_integer const &r)
  {
    return l + big_integer::to_native_real(r);
  }

  native_real operator-(native_big_integer const &l, native_real const &r)
  {
    return big_integer::to_native_real(l) - r;
  }

  native_real operator-(native_real const &l, native_big_integer const &r)
  {
    return l - big_integer::to_native_real(r);
  }

  // native_real operator*(native_big_integer const &l, native_real const &r)
  // {
  //   return l * r;
  // }

  // native_real operator*(native_real const &l, native_big_integer const &r)
  // {
  //   return l * r;
  // }

  // native_real operator/(native_big_integer const &l, native_real const &r)
  // {
  //   return l / r;
  // }

  // native_real operator/(native_real const &l, native_big_integer const &r)
  // {
  //   return l / r;
  // }

  native_bool operator==(native_big_integer const &l, native_real const &r)
  {
    constexpr native_real epsilon = std::numeric_limits<native_real>::epsilon();
    return std::abs(l - r) < epsilon;
  }

  native_bool operator==(native_real const &l, native_big_integer const &r)
  {
    return r == l; // Reuse
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
    return big_integer::to_native_real(l) < r;
  }

  native_bool operator<(native_real const &l, native_big_integer const &r)
  {
    return l < big_integer::to_native_real(r);
  }

  // native_bool operator<=(native_big_integer const &l, native_real const &r)
  // {
  //   return l <= r;
  // }

  // native_bool operator<=(native_real const &l, native_big_integer const &r)
  // {
  //   return l <= r;
  // }


  // native_bool operator>=(native_big_integer const &l, native_real const &r)
  // {
  //   return l >= r;
  // }

  // native_bool operator>=(native_real const &l, native_big_integer const &r)
  // {
  //   return l >= r;
  // }

} // namespace jank::runtime::obj
