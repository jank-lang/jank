#pragma once

#include <cassert>
#include <utility> // move, forward
#include <type_traits>
#include <ostream>

#include <fmt/ostream.h>

#include <jank/type.hpp>

namespace jank
{
  struct none_t
  {
  };

  namespace detail
  {
    template <native_bool Ok, typename T>
    struct result;
  }

  template <typename T>
  struct option
  {
    using storage_type = char[sizeof(T)];
    using value_type = T;

    constexpr option() = default;

    constexpr option(option<T> const &o)
      : set{ o.set }
    {
      if(set)
      {
        new(reinterpret_cast<T *>(data)) T{ o.unwrap_unchecked() };
      }
    }

    constexpr option(option &&o) noexcept
      : set{ std::move(o.set) }
    {
      o.set = false;
      if(set)
      {
        new(reinterpret_cast<T *>(data)) T{ std::move(o.unwrap_unchecked()) };
      }
    }

    constexpr ~option()
    {
      reset();
    }

    template <typename D = T>
    constexpr option(D &&d,
                     std::enable_if_t<std::is_constructible_v<T, D>
                                      && !std::is_same_v<std::decay_t<D>, option<T>>> * = nullptr)
      : set{ true }
    {
      new(reinterpret_cast<T *>(data)) T{ std::forward<D>(d) };
    }

    template <typename D>
    constexpr option(option<D> const &o,
                     std::enable_if_t<std::is_constructible_v<T, D>> * = nullptr)
      : set{ o.set }
    {
      if(set)
      {
        new(reinterpret_cast<T *>(data)) T{ o.unwrap_unchecked() };
      }
    }

    template <typename D>
    requires(std::is_constructible_v<T, D>)
    constexpr option(option<D> &&o)
      : set{ std::move(o.set) }
    {
      if(set)
      {
        new(reinterpret_cast<T *>(data)) T{ std::move(o.unwrap_unchecked()) };
      }
      o.reset();
    }

    constexpr option(none_t const &)
    {
    }

    constexpr option<T> &operator=(option<T> const &rhs)
    {
      if(this == &rhs)
      {
        return *this;
      }
      reset();

      set = rhs.set;
      if(set)
      {
        new(reinterpret_cast<T *>(data)) T{ rhs.unwrap_unchecked() };
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

      set = std::move(rhs.set);
      if(set)
      {
        new(reinterpret_cast<T *>(data)) T{ std::move(rhs.unwrap_unchecked()) };
      }
      rhs.reset();
      return *this;
    }

    constexpr option<T> &operator=(none_t const &)
    {
      reset();
      return *this;
    }

    template <typename D>
    requires(std::is_constructible_v<T, D>)
    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature): It gets this wrong.
    constexpr option<T> &operator=(D &&rhs)
    {
      reset();

      set = true;
      new(reinterpret_cast<T *>(data)) T{ std::forward<D>(rhs) };
      return *this;
    }

    constexpr void reset() noexcept
    {
      if(set)
      {
        reinterpret_cast<T *>(data)->~T();
      }
      set = false;
    }

    constexpr native_bool is_some() const
    {
      return set;
    }

    constexpr native_bool is_none() const
    {
      return !set;
    }

    constexpr T &unwrap()
    {
      /* TODO: Panic fn. */
      assert(set);
      return *reinterpret_cast<T *>(data);
    }

    constexpr T const &unwrap() const
    {
      /* TODO: Panic fn. */
      assert(set);
      return *reinterpret_cast<T const *>(data);
    }

    constexpr T &unwrap_unchecked()
    {
      return *reinterpret_cast<T *>(data);
    }

    constexpr T const &unwrap_unchecked() const
    {
      return *reinterpret_cast<T const *>(data);
    }

    constexpr T &unwrap_or(T &fallback)
    {
      if(set)
      {
        return unwrap_unchecked();
      }
      return fallback;
    }

    /* We don't take const& and return it since that's just asking for lifetime issues. */
    constexpr T unwrap_or(T fallback) const
    {
      if(set)
      {
        return unwrap_unchecked();
      }
      return std::move(fallback);
    }

    template <typename F>
    constexpr auto map(F &&f) const -> option<decltype(f(std::declval<T>()))>
    {
      if(set)
      {
        return f(unwrap_unchecked());
      }
      return none_t{};
    }

    template <typename F>
    constexpr T map_or(T fallback, F &&f) const
    {
      if(set)
      {
        return f(unwrap_unchecked());
      }
      return std::move(fallback);
    }

    constexpr native_bool operator!=(option<T> const &rhs) const
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

    constexpr native_bool operator==(option<T> const &rhs) const
    {
      return !(*this != rhs);
    }

    constexpr native_bool operator!=(T const &rhs) const
    {
      return !set || (*reinterpret_cast<T const *>(data) != rhs);
    }

    constexpr native_bool operator==(T const &rhs) const
    {
      return !(*this != rhs);
    }

    constexpr operator native_bool() const
    {
      return is_some();
    }

    alignas(alignof(T)) storage_type data{};
    native_bool set{};
  };

  template <typename T, typename Decayed = std::decay_t<T>>
  option<Decayed> some(T &&t)
  {
    return { std::forward<T>(t) };
  }

  constexpr inline none_t none = none_t{};

  template <typename T>
  std::ostream &operator<<(std::ostream &os, option<T> const &o)
  {
    if(o.is_none())
    {
      return os << "none";
    }
    return os << "some(" << o.unwrap() << ")";
  }
}

namespace fmt
{
  template <typename T>
  struct formatter<jank::option<T>> : fmt::ostream_formatter
  {
  };
}
