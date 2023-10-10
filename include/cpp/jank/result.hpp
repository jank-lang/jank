#pragma once

#include <iostream>
#include <cstdlib> // std::abort

#include <boost/variant.hpp>

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

    template <bool Ok>
    struct result<Ok, void>
    { };
  }

  inline detail::result<true, void> ok()
  { return {}; }
  template <typename R, typename Decayed = std::decay_t<R>>
  detail::result<true, Decayed> ok(R &&data)
  { return { std::forward<R>(data) }; }
  template <typename E, typename Decayed = std::decay_t<E>>
  detail::result<false, Decayed> err(E &&data)
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
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
    result(T &&t, std::enable_if_t<std::is_constructible_v<R, T>>* = nullptr)
      : data{ std::forward<T>(t) }
    {}
    template <typename T>
    result(T &&t, std::enable_if_t<std::is_constructible_v<E, T>>* = nullptr)
      : data{ std::forward<T>(t) }
    {}
    /* Allow implicit construction from results with compatible constructor args. This allows
     * things like ok(none) for option<R>. */
    template <typename T>
    result(detail::result<true, T> const &t, std::enable_if_t<std::is_constructible_v<R, T>>* = nullptr)
      : data{ t.data }
    {}
    template <typename T>
    result(detail::result<false, T> const &t, std::enable_if_t<std::is_constructible_v<E, T>>* = nullptr)
      : data{ t.data }
    {}

    bool is_ok() const
    { return data.which() == 0; }
    bool is_err() const
    { return data.which() == 1; }

    void assert_ok() const
    {
      if(is_ok())
      { return; }

      auto const &err(expect_err());
      std::cout << "error: expected ok result, but found: " << err << std::endl;
      throw err;
    }

    R const& expect_ok() const
    {
      assert_ok();
      return boost::get<R>(data);
    }
    R* expect_ok_ptr()
    {
      assert_ok();
      return &boost::get<R>(data);
    }
    R const* expect_ok_ptr() const
    {
      assert_ok();
      return &boost::get<R>(data);
    }
    R expect_ok_move()
    {
      assert_ok();
      return std::move(boost::get<R>(data));
    }
    option<R> ok()
    {
      if(is_ok())
      { return some(boost::get<R>(data)); }
      return none;
    }
    E const& expect_err() const
    { return boost::get<E>(data); }
    E* expect_err_ptr()
    { return &boost::get<E>(data); }
    E const* expect_err_ptr() const
    { return &boost::get<E>(data); }
    E expect_err_move()
    { return std::move(boost::get<E>(data)); }
    option<E> err()
    {
      if(is_err())
      { return some(boost::get<E>(data)); }
      return none;
    }

    /* Moves value. */
    R unwrap_move()
    {
      if(!is_ok())
      /* TODO: Panic function. */
      { std::abort(); }
      return std::move(boost::get<R>(data));
    }

    bool operator ==(result const &rhs) const
    { return rhs.data == data; }
    bool operator !=(result const &rhs) const
    { return rhs.data != data; }
    bool operator ==(R const &rhs) const
    { return data.which() == 0 && rhs == boost::get<R>(data); }
    bool operator ==(E const &rhs) const
    { return data.which() == 1 && rhs == boost::get<E>(data); }

    boost::variant<R, E> data;
  };

  struct void_t
  { };

  /* result<void, E> is a special case which doesn't store data for the success case.
   * It still uses a variant, but with a special void_t type which does nothing. The normal
   * "ok" functions for extracting data are gone.
   *
   * This is favorable over an option<E> since result<R, E> is clearly used for error handling. */
  template <typename E>
  struct result<void, E>
  {
    result(detail::result<true, void> &&) : data{ void_t{} }
    {}
    result(detail::result<false, E> &&e) : data{ std::move(e.data) }
    {}
    /* Allow implicit construction of E from its ctor args. */
    template <typename T>
    result(T &&t, std::enable_if_t<std::is_constructible_v<E, T>>* = nullptr)
      : data{ std::forward<T>(t) }
    {}
    /* Allow implicit construction from results with compatible constructor args. This allows
     * things like ok(none) for option<R>. */
    template <typename T>
    result(detail::result<false, T> const &t, std::enable_if_t<std::is_constructible_v<E, T>>* = nullptr)
      : data{ t.data }
    {}

    bool is_ok() const
    { return data.which() == 0; }
    bool is_err() const
    { return data.which() == 1; }

    void assert_ok() const
    {
      if(is_ok())
      { return; }

      auto const &err(expect_err());
      std::cout << "error: expected ok result, but found: " << err << std::endl;
      throw err;
    }
    void expect_ok() const
    { assert_ok(); }

    E const& expect_err() const
    { return boost::get<E>(data); }
    E* expect_err_ptr()
    { return &boost::get<E>(data); }
    E const* expect_err_ptr() const
    { return &boost::get<E>(data); }
    E expect_err_move()
    { return std::move(boost::get<E>(data)); }
    option<E> err()
    {
      if(is_err())
      { return some(boost::get<E>(data)); }
      return none;
    }

    bool operator ==(result const &rhs) const
    { return rhs.data == data; }
    bool operator !=(result const &rhs) const
    { return rhs.data != data; }
    bool operator ==(E const &rhs) const
    { return data.which() == 1 && rhs == boost::get<E>(data); }

    boost::variant<void_t, E> data;
  };

  inline std::ostream& operator <<(std::ostream &os, void_t const &)
  { return os; }

  template <typename R, typename E>
  std::ostream& operator <<(std::ostream &os, result<R, E> const &r)
  {
    if(r.is_ok())
    { return os << "ok(" << boost::get<R>(r.data) << ")"; }
    return os << "err(" << boost::get<E>(r.data) << ")";
  }
}
