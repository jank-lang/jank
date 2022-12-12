#pragma once

#include <cassert>
#include <utility> // move, forward
#include <type_traits>
#include <ostream>

namespace jank
{
  struct none_t
  { };

  namespace detail
  {
    template <bool Ok, typename T>
    struct result;
  }

  template <typename T>
  struct option
  {
    using storage_type = char[sizeof(T)];

    option() = default;
    option(option<T> const &o)
      : set{ o.set }
    {
      if(set)
      { new (reinterpret_cast<T*>(data)) T{ *reinterpret_cast<T const*>(o.data) }; }
    }
    option(option &&o) noexcept
      : set{ std::move(o.set) }
    {
      o.set = false;
      if(set)
      { new (reinterpret_cast<T*>(data)) T{ std::move(*reinterpret_cast<T const*>(o.data)) }; }
    }
    ~option()
    { reset(); }
    template <typename D = T>
    option
    (
      D &&d,
      std::enable_if_t
      <
        std::is_constructible_v<T, D>
        && !std::is_same_v<std::decay_t<D>, option<T>>
      >* = nullptr
    )
      : set{ true }
    { new (reinterpret_cast<T*>(data)) T{ std::forward<D>(d)  }; }
    template <typename D>
    option(option<D> const &o, std::enable_if_t<std::is_constructible_v<T, D>>* = nullptr)
      : set{ o.set }
    {
      if(set)
      { new (reinterpret_cast<T*>(data)) T{ *reinterpret_cast<D const*>(o.data) }; }
    }
    template <typename D>
    option(option<D> &&o, std::enable_if_t<std::is_constructible_v<T, D>>* = nullptr)
      : set{ std::move(o.set) }
    {
      if(set)
      { new (reinterpret_cast<T*>(data)) T{ std::move(*reinterpret_cast<D*>(o.data)) }; }
      o.reset();
    }
    option(none_t const&)
    { }

    option<T>& operator =(option<T> const &rhs)
    {
      if(this == &rhs)
      { return *this; }
      reset();

      set = rhs.set;
      if(set)
      { new (reinterpret_cast<T*>(data)) T{ *reinterpret_cast<T const*>(rhs.data) }; }
      return *this;
    }
    option<T>& operator =(option<T> &&rhs) noexcept
    {
      if(this == &rhs)
      { return *this; }
      reset();

      set = std::move(rhs.set);
      if(set)
      { new (reinterpret_cast<T*>(data)) T{ std::move(*reinterpret_cast<T const*>(rhs.data)) }; }
      rhs.reset();
      return *this;
    }
    option<T>& operator =(none_t const&)
    {
      reset();
      return *this;
    }
    template <typename D>
    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature): It gets this wrong.
    std::enable_if_t<std::is_constructible_v<T, D>, option<T>&> operator =(D &&rhs)
    {
      reset();

      set = true;
      new (reinterpret_cast<T*>(data)) T{ std::forward<D>(rhs) };
      return *this;
    }

    void reset() noexcept
    {
      if(set)
      { reinterpret_cast<T*>(reinterpret_cast<T*>(data))->~T(); }
      set = false;
    }

    bool is_some() const
    { return set; }
    bool is_none() const
    { return !set; }

    T& unwrap()
    {
      /* TODO: Panic fn. */
      assert(set);
      return *reinterpret_cast<T*>(data);
    }
    T const& unwrap() const
    {
      /* TODO: Panic fn. */
      assert(set);
      return *reinterpret_cast<T const*>(data);
    }
    T& unwrap_or(T &fallback)
    {
      if(set)
      { return *reinterpret_cast<T*>(data); }
      return fallback;
    }
    /* We don't take const& and return it since that's just asking for lifetime issues. */
    T unwrap_or(T fallback) const
    {
      if(set)
      { return *reinterpret_cast<T const*>(data); }
      return std::move(fallback);
    }

    bool operator !=(option<T> const &rhs) const
    {
      if(set != rhs.set)
      { return true; }
      else if(set)
      { return *reinterpret_cast<T const*>(data) != *reinterpret_cast<T const*>(rhs.data); }

      return true;
    }
    bool operator ==(option<T> const &rhs) const
    { return !(*this != rhs); }

    bool operator !=(T const &rhs) const
    { return !set || (*reinterpret_cast<T const*>(data) != rhs); }
    bool operator ==(T const &rhs) const
    { return !(*this != rhs); }

    bool set{};
    alignas(alignof(T)) storage_type data{};
  };

  template <typename T, typename Decayed = std::decay_t<T>>
  option<Decayed> some(T &&t)
  { return { std::forward<T>(t) }; }
  inline constexpr none_t none = none_t{};

  template <typename T>
  std::ostream& operator<<(std::ostream &os, option<T> const &o)
  {
    if(o.is_none())
    { return os << "none"; }
    return os << "some(" << o.unwrap() << ")";
  }
}
