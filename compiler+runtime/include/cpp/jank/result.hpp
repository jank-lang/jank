#pragma once

#include <variant>

#include <jtl/option.hpp>

#include <jank/native_persistent_string.hpp>
#include <jank/util/type_name.hpp>

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
    static_assert(!std::same_as<R, E>, "Result and error type must be different.");

    constexpr result(detail::result<true, R> &&r)
      : data{ R{ std::move(r.data) } }
    {
    }

    constexpr result(detail::result<false, E> &&e)
      : data{ E{ std::move(e.data) } }
    {
    }

    /* Allow implicit construction of R and E from their ctor args. */
    template <typename T>
    requires std::is_constructible_v<R, T>
    constexpr result(T &&t)
      : data{ std::forward<T>(t) }
    {
    }

    template <typename T>
    requires std::is_constructible_v<E, T>
    constexpr result(T &&t)
      : data{ std::forward<T>(t) }
    {
    }

    /* Allow implicit construction from results with compatible constructor args. This allows
     * things like ok(none) for jtl::option<R>. */
    template <typename T>
    constexpr result(detail::result<true, T> const &t,
                     std::enable_if_t<std::is_constructible_v<R, T>> * = nullptr)
      : data{ R{ t.data } }
    {
    }

    template <typename T>
    constexpr result(detail::result<false, T> const &t,
                     std::enable_if_t<std::is_constructible_v<E, T>> * = nullptr)
      : data{ E{ t.data } }
    {
    }

    constexpr bool is_ok() const
    {
      return data.index() == 0;
    }

    constexpr bool is_err() const
    {
      return data.index() == 1;
    }

    constexpr void assert_ok() const
    {
      if(is_ok())
      {
        return;
      }

      /* TODO: Update all of these throws to throw a consistent type, regardless of the
       * error type. This simplifies our catching logic. */
      throw expect_err();
    }

    constexpr R const &expect_ok() const
    {
      assert_ok();
      return std::get<R>(data);
    }

    constexpr R *expect_ok_ptr()
    {
      assert_ok();
      return &std::get<R>(data);
    }

    constexpr R const *expect_ok_ptr() const
    {
      assert_ok();
      return &std::get<R>(data);
    }

    constexpr R expect_ok_move()
    {
      assert_ok();
      return std::move(std::get<R>(data));
    }

    constexpr jtl::option<R> ok()
    {
      if(is_ok())
      {
        return some(std::get<R>(data));
      }
      return none;
    }

    constexpr E const &expect_err() const
    {
      return std::get<E>(data);
    }

    constexpr E *expect_err_ptr()
    {
      return &std::get<E>(data);
    }

    constexpr E const *expect_err_ptr() const
    {
      return &std::get<E>(data);
    }

    constexpr E expect_err_move()
    {
      return std::move(std::get<E>(data));
    }

    constexpr jtl::option<E> err()
    {
      if(is_err())
      {
        return some(std::get<E>(data));
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
      return std::move(std::get<R>(data));
    }

    constexpr R &unwrap_or(R &fallback)
    {
      if(is_ok())
      {
        return std::get<R>(data);
      }
      return fallback;
    }

    /* We don't take const& and return it since that's just asking for lifetime issues. */
    constexpr R unwrap_or(R fallback) const
    {
      if(is_ok())
      {
        return std::get<R>(data);
      }
      return std::move(fallback);
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
      return data.index() == 0 && rhs == std::get<R>(data);
    }

    constexpr bool operator==(E const &rhs) const
    {
      return data.index() == 1 && rhs == std::get<E>(data);
    }

    std::variant<R, E> data;
  };

  struct void_t
  {
  };

  /* result<void, E> is a special case which doesn't store data for the success case.
   * It still uses a variant, but with a special void_t type which does nothing. The normal
   * "ok" functions for extracting data are gone.
   *
   * This is favorable over an jtl::option<E> since result<R, E> is clearly used for error handling. */
  template <typename E>
  struct [[nodiscard]] result<void, E>
  {
    constexpr result(detail::result<true, void> &&)
      : data{ void_t{} }
    {
    }

    constexpr result(detail::result<false, E> &&e)
      : data{ std::move(e).data }
    {
    }

    /* Allow implicit construction of E from its ctor args. */
    template <typename T>
    constexpr result(T &&t, std::enable_if_t<std::is_constructible_v<E, T>> * = nullptr)
      : data{ std::forward<T>(t) }
    {
    }

    /* Allow implicit construction from results with compatible constructor args. This allows
     * things like ok(none) for jtl::option<R>. */
    template <typename T>
    constexpr result(detail::result<false, T> const &t,
                     std::enable_if_t<std::is_constructible_v<E, T>> * = nullptr)
      : data{ t.data }
    {
    }

    constexpr bool is_ok() const
    {
      return data.index() == 0;
    }

    constexpr bool is_err() const
    {
      return data.index() == 1;
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
      return std::get<E>(data);
    }

    constexpr E *expect_err_ptr()
    {
      return &std::get<E>(data);
    }

    constexpr E const *expect_err_ptr() const
    {
      return &std::get<E>(data);
    }

    constexpr E expect_err_move()
    {
      return std::move(std::get<E>(data));
    }

    constexpr jtl::option<E> err()
    {
      if(is_err())
      {
        return some(std::get<E>(data));
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
      return data.index() == 1 && rhs == std::get<E>(data);
    }

    std::variant<void_t, E> data;
  };

  constexpr std::ostream &operator<<(std::ostream &os, void_t const &)
  {
    return os;
  }

  template <typename R, typename E>
  constexpr std::ostream &operator<<(std::ostream &os, result<R, E> const &r)
  {
    /* It's possible that we don't have an op<< overload for R or E, so we
     * fall back to just rendering the type name instead. */
    /* TODO: Datafy concept, a la Rust's Debug trait. */
    if(r.is_ok())
    {
      if constexpr(requires(R t) { os << t; })
      {
        return os << "ok(" << std::get<R>(r.data) << ")";
      }
      else
      {
        return os << "ok(" << util::type_name<R>() << ")";
      }
    }

    if constexpr(requires(E t) { os << t; })
    {
      return os << "err(" << std::get<E>(r.data) << ")";
    }
    else
    {
      return os << "err(" << util::type_name<E>() << ")";
    }
  }

  template <typename R>
  using string_result = result<R, native_persistent_string>;
}
