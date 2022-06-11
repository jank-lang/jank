#pragma once

#include <variant>
#include <ostream>
#include <cstdlib> // std::abort

#include <jank/option.hpp>

namespace jank
{
  namespace detail
  {
    template <bool Ok, typename T>
    struct result
    {
      result(T const &t) : data{ t }
      { }
      result(T &&t) : data{ std::move(t) }
      { }

      T data;
    };
  }

  template <typename R, typename Decayed = std::decay_t<R>>
  detail::result<true, Decayed> ok(R &&data)
  { return { std::forward<R>(data) }; }
  template <typename E, typename Decayed = std::decay_t<E>>
  detail::result<false, Decayed> err(E &&data)
  { return { std::forward<E>(data) }; }

  template <typename R, typename E>
  struct result
  {
    result(detail::result<true, R> &&r) : data{ std::move(r.data) }
    {}
    result(detail::result<false, E> &&e) : data{ std::move(e.data) }
    {}
    /* Allow implicit construction of R and E from their ctor args. */
    template <typename T>
    result(T &&t, std::enable_if_t<std::is_constructible_v<R, T>>* = 0)
      : data{ std::forward<T>(t) }
    {}
    template <typename T>
    result(T &&t, std::enable_if_t<std::is_constructible_v<E, T>>* = 0)
      : data{ std::forward<T>(t) }
    {}
    /* Allow implicit construction from results with compatible constructor args. This allows
     * things like ok(none) for option<some<R>>. */
    template <typename T>
    result(detail::result<true, T> const &t, std::enable_if_t<std::is_constructible_v<R, T>>* = 0)
      : data{ t.data }
    {}
    template <typename T>
    result(detail::result<false, T> const &t, std::enable_if_t<std::is_constructible_v<E, T>>* = 0)
      : data{ t.data }
    {}

    bool is_ok() const
    { return data.index() == 0; }
    bool is_err() const
    { return data.index() == 1; }

    R expect_ok() const
    { return std::get<0>(data); }
    R* expect_ok_ptr()
    { return &std::get<0>(data); }
    R const* expect_ok_ptr() const
    { return &std::get<0>(data); }
    R expect_ok_move()
    { return std::move(std::get<0>(data)); }
    option<R> ok()
    {
      if(is_ok())
      { return some(std::get<0>(data)); }
      return none;
    }
    E expect_err() const
    { return std::get<1>(data); }
    E* expect_err_ptr()
    { return &std::get<1>(data); }
    E const* expect_err_ptr() const
    { return &std::get<1>(data); }
    E expect_err_move()
    { return std::move(std::get<1>(data)); }
    option<E> err()
    {
      if(is_err())
      { return some(std::get<1>(data)); }
      return none;
    }

    /* Moves value. */
    R unwrap()
    {
      if(!is_ok())
      /* TODO: Panic function. */
      { std::abort(); }
      return std::move(std::get<0>(data));
    }

    bool operator ==(result const &rhs) const
    { return rhs.data == data; }
    bool operator !=(result const &rhs) const
    { return rhs.data != data; }
    bool operator ==(R const &rhs) const
    { return data.index() == 0 && rhs == std::get<0>(data); }
    bool operator ==(E const &rhs) const
    { return data.index() == 1 && rhs == std::get<1>(data); }

    std::variant<R, E> data;
  };

  template <typename R, typename E>
  std::ostream& operator <<(std::ostream &os, result<R, E> const &r)
  {
    if(r.is_ok())
    { return os << std::get<0>(r.data); }
    return os << std::get<1>(r.data);
  }
}
