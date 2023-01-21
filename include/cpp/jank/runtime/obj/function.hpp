#pragma once

#include <iostream>

#include <jank/runtime/behavior/callable.hpp>
#include <jank/runtime/behavior/metadatable.hpp>

namespace jank::runtime::obj
{
  namespace detail
  {
    struct function_type
    {
      template <typename T>
      using value_type = std::function<T>;

      function_type() = default;
      template <typename R, typename... Args>
      function_type(R (* const f)(Args...)) : function_type(value_type<R (Args...)>{ f })
      { }
      template <typename R, typename... Args>
      function_type(value_type<R (Args...)> &&f) : value{ std::move(f) }
      { }
      template <typename R, typename... Args>
      function_type(value_type<R (Args...)> const &f) : value{ f }
      { }

      template <typename F>
      F const* get() const
      { return std::any_cast<F>(&value); }

      std::any value;
    };
  }

  template <size_t Arity>
  struct invalid_arity
  { };

  struct function : object, behavior::callable, behavior::metadatable
  {
    function() = default;
    function(function &&) = default;
    function(function const &) = default;
    function(detail::function_type &&d);
    function(detail::function_type const &d);
    ~function() = default;

    static native_box<function> create(detail::function_type const &d);

    native_bool equal(object const &) const override;
    native_string to_string() const override;
    void to_string(fmt::memory_buffer &buff) const override;
    native_integer to_hash() const override;

    function const* as_function() const override;
    behavior::callable const* as_callable() const override;

    object_ptr call() const override;
    object_ptr call(object_ptr) const override;
    object_ptr call(object_ptr, object_ptr) const override;
    object_ptr call(object_ptr, object_ptr, object_ptr) const override;
    object_ptr call(object_ptr, object_ptr, object_ptr, object_ptr) const override;
    object_ptr call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr) const override;
    object_ptr call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr) const override;
    object_ptr call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr) const override;
    object_ptr call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr) const override;
    object_ptr call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr) const override;
    object_ptr call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr) const override;

    object_ptr with_meta(object_ptr m) const override;
    behavior::metadatable const* as_metadatable() const override;

    detail::function_type data;
  };
  using function_ptr = native_box<function>;

  namespace detail
  {
    template <typename F, typename... Args>
    object_ptr invoke(F const &f, Args &&... args)
    {
      if constexpr(std::is_function_v<std::remove_pointer_t<std::decay_t<decltype(f)>>>)
      { return f(std::forward<Args>(args)...); }
      else
      {
        auto const * const c((*f)->as_callable());

        if(c)
        { return c->call(std::forward<Args>(args)...); }
        else
        {
          /* TODO: Throw error. */
          std::cout << "(invoke) object is not callable: " << **f << std::endl;
          return JANK_NIL;
        }
      }
    }
  }
}
