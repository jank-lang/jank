#pragma once

#include <cmath>
#include <random>

#include <prelude/object.hpp>

namespace jank
{
  inline object rand()
  {
    static std::uniform_real_distribution<detail::real> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
  }

  /* + */
  inline object _gen_plus_(object const &l, object const &r)
  {
    return l.visit_with
    (
      [&](auto const &l_data, auto const &r_data) -> object
      {
        using L = std::decay_t<decltype(l_data)>;
        using R = std::decay_t<decltype(r_data)>;

        /* TODO: Trait for is_number_v */
        if constexpr((std::is_same_v<L, detail::integer> || std::is_same_v<L, detail::real>)
                     && (std::is_same_v<R, detail::integer> || std::is_same_v<R, detail::real>))
        { return object{ l_data + r_data }; }
        else
        {
          /* TODO: Throw an error. */
          std::cout << "not a number" << std::endl;
          return JANK_NIL;
        }
      },
      r
    );
  }

  /* - */
  inline object _gen_minus_(object const &l, object const &r)
  {
    return l.visit_with
    (
      [&](auto const &l_data, auto const &r_data) -> object
      {
        using L = std::decay_t<decltype(l_data)>;
        using R = std::decay_t<decltype(r_data)>;

        /* TODO: Trait for is_number_v */
        if constexpr((std::is_same_v<L, detail::integer> || std::is_same_v<L, detail::real>)
                     && (std::is_same_v<R, detail::integer> || std::is_same_v<R, detail::real>))
        { return object{ l_data - r_data }; }
        else
        {
          /* TODO: Throw an error. */
          std::cout << "not a number" << std::endl;
          return JANK_NIL;
        }
      },
      r
    );
  }

  /* < */
  inline object _gen_less_(object const &l, object const &r)
  {
    return l.visit_with
    (
      [&](auto const &l_data, auto const &r_data) -> object
      {
        using L = std::decay_t<decltype(l_data)>;
        using R = std::decay_t<decltype(r_data)>;

        /* TODO: Trait for is_number_v */
        if constexpr((std::is_same_v<L, detail::integer> || std::is_same_v<L, detail::real>)
                     && (std::is_same_v<R, detail::integer> || std::is_same_v<R, detail::real>))
        { return object{ l_data < r_data }; }
        else
        {
          /* TODO: Throw an error. */
          std::cout << "not a number" << std::endl;
          return JANK_NIL;
        }
      },
      r
    );
  }

  /* * */
  inline object _gen_asterisk_(object const &l, object const &r)
  {
    return l.visit_with
    (
      [&](auto const &l_data, auto const &r_data) -> object
      {
        using L = std::decay_t<decltype(l_data)>;
        using R = std::decay_t<decltype(r_data)>;

        /* TODO: Trait for is_number_v */
        if constexpr((std::is_same_v<L, detail::integer> || std::is_same_v<L, detail::real>)
                     && (std::is_same_v<R, detail::integer> || std::is_same_v<R, detail::real>))
        { return object{ l_data * r_data }; }
        else
        {
          /* TODO: Throw an error. */
          std::cout << "not a number" << std::endl;
          return JANK_NIL;
        }
      },
      r
    );
  }

  /* / */
  /* TODO: Handle naming this / */
  inline object div(object const &l, object const &r)
  {
    return l.visit_with
    (
      [&](auto const &l_data, auto const &r_data) -> object
      {
        using L = std::decay_t<decltype(l_data)>;
        using R = std::decay_t<decltype(r_data)>;

        /* TODO: Trait for is_number_v */
        if constexpr((std::is_same_v<L, detail::integer> || std::is_same_v<L, detail::real>)
                     && (std::is_same_v<R, detail::integer> || std::is_same_v<R, detail::real>))
        { return object{ l_data / r_data }; }
        else
        {
          /* TODO: Throw an error. */
          std::cout << "not a number" << std::endl;
          return JANK_NIL;
        }
      },
      r
    );
  }

  /* ->int */
  inline object _gen_minus__gen_greater_int(object const &o)
  {
    return o.visit
    (
      [&](auto const &data) -> object
      {
        using T = std::decay_t<decltype(data)>;

        /* TODO: Trait for is_number_v */
        if constexpr(std::is_same_v<T, detail::integer>)
        { return data; }
        if constexpr(std::is_same_v<T, detail::real>)
        { return detail::integer(data); }
        else
        {
          /* TODO: Throw an error. */
          std::cout << "not a number" << std::endl;
          return JANK_NIL;
        }
      }
    );
  }

  /* ->float */
  inline object _gen_minus__gen_greater_float(object const &o)
  {
    return o.visit
    (
      [&](auto const &data) -> object
      {
        using T = std::decay_t<decltype(data)>;

        /* TODO: Trait for is_number_v */
        if constexpr(std::is_same_v<T, detail::real>)
        { return data; }
        if constexpr(std::is_same_v<T, detail::integer>)
        { return detail::real(data); }
        else
        {
          /* TODO: Throw an error. */
          std::cout << "not a number" << std::endl;
          return JANK_NIL;
        }
      }
    );
  }

  inline object inc(object const &o)
  {
    return o.visit
    (
      [&](auto const &data) -> object
      {
        using T = std::decay_t<decltype(data)>;

        /* TODO: Trait for is_number_v */
        if constexpr(std::is_same_v<T, detail::integer> || std::is_same_v<T, detail::real>)
        { return object{ data + 1 }; }
        else
        {
          /* TODO: Throw an error. */
          std::cout << "not a number" << std::endl;
          return JANK_NIL;
        }
      }
    );
  }

  inline object dec(object const &o)
  {
    return o.visit
    (
      [&](auto const &data) -> object
      {
        using T = std::decay_t<decltype(data)>;

        /* TODO: Trait for is_number_v */
        if constexpr(std::is_same_v<T, detail::integer> || std::is_same_v<T, detail::real>)
        { return object{ data - 1 }; }
        else
        {
          /* TODO: Throw an error. */
          std::cout << "not a number" << std::endl;
          return JANK_NIL;
        }
      }
    );
  }

  inline object sqrt(object const &o)
  {
    return o.visit
    (
      [&](auto const &data) -> object
      {
        using T = std::decay_t<decltype(data)>;

        /* TODO: Trait for is_number_v */
        if constexpr(std::is_same_v<T, detail::integer> || std::is_same_v<T, detail::real>)
        { return object{ std::sqrt(data) }; }
        else
        {
          /* TODO: Throw an error. */
          std::cout << "not a number" << std::endl;
          return JANK_NIL;
        }
      }
    );
  }
}
