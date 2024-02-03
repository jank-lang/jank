#pragma once

#include <boost/variant.hpp>

#include <jank/option.hpp>

namespace jank
{
  namespace detail
  {
    template <bool Ok, typename T>
    struct result
    {
      constexpr result(T const &t)
        : data{ t }
      {
      }

      constexpr result(T &&t)
        : data{ std::move(t) }
      {
      }

      T data;
    };

    template <bool Ok>
    struct result<Ok, void>
    {
    };
  }

  constexpr detail::result<true, void> ok()
  {
    return {};
  }

  template <typename R, typename Decayed = std::decay_t<R>>
  constexpr detail::result<true, Decayed> ok(R &&data)
  {
    return { std::forward<R>(data) };
  }
  template <typename E, typename Decayed = std::decay_t<E>>
  constexpr detail::result<false, Decayed> err(E &&data)
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  {
    return { std::forward<E>(data) };
  }

  template <typename R, typename E>
  struct [[nodiscard]] result
  {
    constexpr result(detail::result<true, R> &&r)
      : data{ std::move(r.data) }
    {
    }

    constexpr result(detail::result<false, E> &&e)
      : data{ std::move(e.data) }
    {
    }

    /* Allow implicit construction of R and E from their ctor args. */
    template <typename T>
    constexpr result(T &&t, std::enable_if_t<std::is_constructible_v<R, T>> * = nullptr)
      : data{ std::forward<T>(t) }
    {
    }

    template <typename T>
    constexpr result(T &&t, std::enable_if_t<std::is_constructible_v<E, T>> * = nullptr)
      : data{ std::forward<T>(t) }
    {
    }

    /* Allow implicit construction from results with compatible constructor args. This allows
     * things like ok(none) for option<R>. */
    template <typename T>
    constexpr result(detail::result<true, T> const &t,
                     std::enable_if_t<std::is_constructible_v<R, T>> * = nullptr)
      : data{ t.data }
    {
    }

    template <typename T>
    constexpr result(detail::result<false, T> const &t,
                     std::enable_if_t<std::is_constructible_v<E, T>> * = nullptr)
      : data{ t.data }
    {
    }

    constexpr bool is_ok() const
    {
      return data.which() == 0;
    }

    constexpr bool is_err() const
    {
      return data.which() == 1;
    }

    constexpr void assert_ok() const
    {
      if(is_ok())
      {
        return;
      }

      throw expect_err();
    }

    constexpr R const &expect_ok() const
    {
      assert_ok();
      return boost::get<R>(data);
    }

    constexpr R *expect_ok_ptr()
    {
      assert_ok();
      return &boost::get<R>(data);
    }

    constexpr R const *expect_ok_ptr() const
    {
      assert_ok();
      return &boost::get<R>(data);
    }

    constexpr R expect_ok_move()
    {
      assert_ok();
      return std::move(boost::get<R>(data));
    }

    constexpr option<R> ok()
    {
      if(is_ok())
      {
        return some(boost::get<R>(data));
      }
      return none;
    }

    constexpr E const &expect_err() const
    {
      return boost::get<E>(data);
    }

    constexpr E *expect_err_ptr()
    {
      return &boost::get<E>(data);
    }

    constexpr E const *expect_err_ptr() const
    {
      return &boost::get<E>(data);
    }

    constexpr E expect_err_move()
    {
      return std::move(boost::get<E>(data));
    }

    constexpr option<E> err()
    {
      if(is_err())
      {
        return some(boost::get<E>(data));
      }
      return none;
    }

    /* Moves value. */
    constexpr R unwrap_move()
    {
      if(!is_ok())
      /* TODO: Panic function. */
      {
        throw expect_err();
      }
      return std::move(boost::get<R>(data));
    }

    constexpr bool operator==(result const &rhs) const
    {
      return rhs.data == data;
    }

    constexpr bool operator!=(result const &rhs) const
    {
      return rhs.data != data;
    }

    constexpr bool operator==(R const &rhs) const
    {
      return data.which() == 0 && rhs == boost::get<R>(data);
    }

    constexpr bool operator==(E const &rhs) const
    {
      return data.which() == 1 && rhs == boost::get<E>(data);
    }

    boost::variant<R, E> data;
  };

  struct void_t
  {
  };

  /* result<void, E> is a special case which doesn't store data for the success case.
   * It still uses a variant, but with a special void_t type which does nothing. The normal
   * "ok" functions for extracting data are gone.
   *
   * This is favorable over an option<E> since result<R, E> is clearly used for error handling. */
  template <typename E>
  struct [[nodiscard]] result<void, E>
  {
    constexpr result(detail::result<true, void> &&)
      : data{ void_t{} }
    {
    }

    constexpr result(detail::result<false, E> &&e)
      : data{ std::move(e.data) }
    {
    }

    /* Allow implicit construction of E from its ctor args. */
    template <typename T>
    constexpr result(T &&t, std::enable_if_t<std::is_constructible_v<E, T>> * = nullptr)
      : data{ std::forward<T>(t) }
    {
    }

    /* Allow implicit construction from results with compatible constructor args. This allows
     * things like ok(none) for option<R>. */
    template <typename T>
    constexpr result(detail::result<false, T> const &t,
                     std::enable_if_t<std::is_constructible_v<E, T>> * = nullptr)
      : data{ t.data }
    {
    }

    constexpr bool is_ok() const
    {
      return data.which() == 0;
    }

    constexpr bool is_err() const
    {
      return data.which() == 1;
    }

    constexpr void assert_ok() const
    {
      if(is_ok())
      {
        return;
      }

      throw expect_err();
    }

    constexpr void expect_ok() const
    {
      assert_ok();
    }

    constexpr E const &expect_err() const
    {
      return boost::get<E>(data);
    }

    constexpr E *expect_err_ptr()
    {
      return &boost::get<E>(data);
    }

    constexpr E const *expect_err_ptr() const
    {
      return &boost::get<E>(data);
    }

    constexpr E expect_err_move()
    {
      return std::move(boost::get<E>(data));
    }

    constexpr option<E> err()
    {
      if(is_err())
      {
        return some(boost::get<E>(data));
      }
      return none;
    }

    constexpr bool operator==(result const &rhs) const
    {
      return rhs.data == data;
    }

    constexpr bool operator!=(result const &rhs) const
    {
      return rhs.data != data;
    }

    constexpr bool operator==(E const &rhs) const
    {
      return data.which() == 1 && rhs == boost::get<E>(data);
    }

    boost::variant<void_t, E> data;
  };

  constexpr std::ostream &operator<<(std::ostream &os, void_t const &)
  {
    return os;
  }

  template <typename R, typename E>
  constexpr std::ostream &operator<<(std::ostream &os, result<R, E> const &r)
  {
    if(r.is_ok())
    {
      return os << "ok(" << boost::get<R>(r.data) << ")";
    }
    return os << "err(" << boost::get<E>(r.data) << ")";
  }

  template <typename R>
  using string_result = result<R, native_persistent_string>;
}
