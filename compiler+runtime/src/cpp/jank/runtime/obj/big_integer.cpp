#include <jank/runtime/obj/big_integer.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/obj/ratio.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/util/fmt.hpp>

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

  native_hash big_integer::to_hash() const
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

  native_integer big_integer::to_integer() const
  {
    try
    {
      return data.template convert_to<native_integer>();
    }
    catch(std::overflow_error const &)
    {
      throw std::runtime_error("BigInteger value out of range for native_integer");
    }
    catch(std::exception const &e)
    {
      throw std::runtime_error(
        util::format("Error converting BigInteger to native_integer: {}", e.what()));
    }
  }

  native_real big_integer::to_real() const
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

} // namespace jank::runtime::obj
