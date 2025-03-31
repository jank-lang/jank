#pragma once

#include <jtl/assert.hpp>
#include <jtl/storage.hpp>

namespace jtl
{
  struct none_t
  {
  };

  namespace detail
  {
    template <bool Ok, typename T>
    struct result;
  }

  template <typename T>
  struct option
  {
    using value_type = T;

    constexpr option() noexcept = default;

    constexpr option(option<T> const &o) noexcept
      : set{ o.set }
    {
      if(set)
      {
        data.construct(o.unwrap_unchecked());
      }
    }

    constexpr option(option &&o) noexcept
      : set{ jtl::move(o.set) }
    {
      o.set = false;
      if(set)
      {
        data.construct(jtl::move(o.unwrap_unchecked()));
      }
    }

    constexpr ~option()
    {
      reset();
    }

    template <typename D = T>
    requires(jtl::is_constructible<T, D> && !jtl::is_same<jtl::decay_t<D>, option<T>>)
    constexpr option(D &&d) noexcept
      : set{ true }
    {
      data.construct(jtl::forward<D>(d));
    }

    template <typename D>
    requires(jtl::is_constructible<T, D>)
    constexpr option(option<D> const &o) noexcept
      : set{ o.set }
    {
      if(set)
      {
        data.construct(o.unwrap_unchecked());
      }
    }

    template <typename D>
    requires(jtl::is_constructible<T, D>)
    constexpr option(option<D> &&o) noexcept
      : set{ jtl::move(o.set) }
    {
      if(set)
      {
        data.construct(jtl::move(o.unwrap_unchecked()));
      }
      o.reset();
    }

    constexpr option(none_t const &) noexcept
    {
    }

    constexpr option<T> &operator=(option<T> const &rhs) noexcept
    {
      if(this == &rhs)
      {
        return *this;
      }
      reset();

      set = rhs.set;
      if(set)
      {
        data.construct(rhs.unwrap_unchecked());
      }
      return *this;
    }

    constexpr option<T> &operator=(option<T> &&rhs) noexcept
    {
      if(this == &rhs)
      {
        return *this;
      }
      reset();

      set = jtl::move(rhs.set);
      if(set)
      {
        data.construct(jtl::move(rhs.unwrap_unchecked()));
      }
      rhs.reset();
      return *this;
    }

    constexpr option<T> &operator=(none_t const &) noexcept
    {
      reset();
      return *this;
    }

    template <typename D>
    requires(jtl::is_constructible<T, D>)
    constexpr option &operator=(D &&rhs) noexcept
    {
      reset();

      set = true;
      data.construct(jtl::forward<D>(rhs));
      return *this;
    }

    constexpr void reset() noexcept
    {
      if(set)
      {
        data.destruct();
      }
      set = false;
    }

    constexpr bool is_some() const noexcept
    {
      return set;
    }

    constexpr bool is_none() const noexcept
    {
      return !set;
    }

    constexpr T &unwrap() noexcept
    {
      jank_debug_assert(set);
      return *data;
    }

    constexpr T const &unwrap() const noexcept
    {
      jank_debug_assert(set);
      return *data;
    }

    constexpr T &unwrap_unchecked() noexcept
    {
      return *data;
    }

    constexpr T const &unwrap_unchecked() const noexcept
    {
      return *data;
    }

    constexpr T &unwrap_or(T &fallback) noexcept
    {
      if(set)
      {
        return unwrap_unchecked();
      }
      return fallback;
    }

    /* We don't take const& and return it since that's just asking for lifetime issues. */
    constexpr T unwrap_or(T fallback) const noexcept
    {
      if(set)
      {
        return unwrap_unchecked();
      }
      return jtl::move(fallback);
    }

    template <typename F>
    constexpr auto map(F const &f) const noexcept -> option<decltype(f(jtl::declval<T>()))>
    {
      if(set)
      {
        return f(unwrap_unchecked());
      }
      return none_t{};
    }

    template <typename F>
    constexpr T map_or(T fallback, F const &f) const noexcept
    {
      if(set)
      {
        return f(unwrap_unchecked());
      }
      return jtl::move(fallback);
    }

    /* TODO: noexcept when we can. */
    constexpr bool operator!=(option<T> const &rhs) const
    {
      if(set != rhs.set)
      {
        return true;
      }
      else if(set)
      {
        return unwrap_unchecked() != rhs.unwrap_unchecked();
      }

      return false;
    }

    constexpr bool operator==(option<T> const &rhs) const noexcept
    {
      return !(*this != rhs);
    }

    constexpr bool operator!=(T const &rhs) const noexcept
    {
      return !set || (*data.ptr() != rhs);
    }

    constexpr bool operator==(T const &rhs) const noexcept
    {
      return !(*this != rhs);
    }

    constexpr operator bool() const noexcept
    {
      return is_some();
    }

    jtl::storage<T> data;
    bool set{};
  };

  template <typename T, typename Decayed = jtl::decay_t<T>>
  constexpr option<Decayed> some(T &&t) noexcept
  {
    return { jtl::forward<T>(t) };
  }

  constexpr inline none_t none = none_t{};
}

namespace jank
{
  using jtl::some;
  using jtl::none;
}
