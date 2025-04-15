#pragma once

#include <iostream>
#include <any>

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/callable.hpp>
#include <jank/runtime/behavior/metadatable.hpp>

namespace jank::runtime
{
  namespace obj::detail
  {
    /* This must be constructed with a std::function object, since we need put it into a
     * std::any and pull it out based on how we call it. i.e. if we try to call it with
     * one param, we try to grab it from the std::any as a std::function<object_ref (object_ref)>.
     *
     * This means you can't just dump a lambda into this. Build a std::function from it first. */
    struct function_type
    {
      template <typename T>
      using value_type = std::function<T>;

      function_type() = default;

      template <typename R, typename... Args>
      function_type(R (* const f)(Args...))
        : function_type{ value_type<R(Args...)>{ f } }
      {
      }

      template <typename R, typename... Args>
      function_type(value_type<R(Args...)> &&f)
        : value{ std::move(f) }
      {
      }

      template <typename R, typename... Args>
      function_type(value_type<R(Args...)> const &f)
        : value{ f }
      {
      }

      template <typename F>
      F const *get() const
      {
        return std::any_cast<F>(&value);
      }

      std::any value;
    };
  }

  template <size_t Arity>
  struct invalid_arity : std::runtime_error
  {
    invalid_arity(jtl::immutable_string const &name)
      : std::runtime_error{ std::string{ "invalid call to " } + name + " with "
                            + std::to_string(Arity) + " args provided" }
    {
    }
  };

  namespace obj
  {
    using native_function_wrapper_ref = jtl::oref<struct native_function_wrapper>;

    struct native_function_wrapper
      : gc
      , behavior::callable
    {
      static constexpr object_type obj_type{ object_type::native_function_wrapper };
      static constexpr native_bool pointer_free{ false };

      native_function_wrapper() = default;
      native_function_wrapper(native_function_wrapper &&) noexcept = default;
      native_function_wrapper(native_function_wrapper const &) = default;
      native_function_wrapper(obj::detail::function_type &&d);
      native_function_wrapper(obj::detail::function_type const &d);

      /* behavior::object_like */
      native_bool equal(object const &) const;
      jtl::immutable_string to_string() const;
      void to_string(util::string_builder &buff) const;
      jtl::immutable_string to_code_string() const;
      native_hash to_hash() const;

      /* behavior::callable */
      object_ref call() final;
      object_ref call(object_ref) final;
      object_ref call(object_ref, object_ref) final;
      object_ref call(object_ref, object_ref, object_ref) final;
      object_ref call(object_ref, object_ref, object_ref, object_ref) final;
      object_ref call(object_ref, object_ref, object_ref, object_ref, object_ref) final;
      object_ref call(object_ref, object_ref, object_ref, object_ref, object_ref, object_ref) final;
      object_ref
        call(object_ref, object_ref, object_ref, object_ref, object_ref, object_ref, object_ref)
          final;
      object_ref call(object_ref,
                      object_ref,
                      object_ref,
                      object_ref,
                      object_ref,
                      object_ref,
                      object_ref,
                      object_ref) final;
      object_ref call(object_ref,
                      object_ref,
                      object_ref,
                      object_ref,
                      object_ref,
                      object_ref,
                      object_ref,
                      object_ref,
                      object_ref) final;
      object_ref call(object_ref,
                      object_ref,
                      object_ref,
                      object_ref,
                      object_ref,
                      object_ref,
                      object_ref,
                      object_ref,
                      object_ref,
                      object_ref) final;

      object_ref this_object_ref() final;

      /* behavior::metadatable */
      native_function_wrapper_ref with_meta(object_ref m) const;

      object base{ obj_type };
      obj::detail::function_type data{};
      jtl::option<object_ref> meta;
    };
  }

  namespace detail
  {
    /* TODO: Is this needed, given dynamic_call? */
    template <typename F, typename... Args>
    object_ref invoke(F const &f, Args &&...args)
    {
      if constexpr(std::is_function_v<std::remove_pointer_t<std::decay_t<decltype(f)>>>)
      {
        return f(std::forward<Args>(args)...);
      }
      else
      {
        auto const * const c((*f)->as_callable());

        if(c)
        {
          return c->call(std::forward<Args>(args)...);
        }
        else
        {
          /* TODO: Better error. */
          std::cout << "(invoke) object is not callable: " << **f << std::endl;
          throw std::runtime_error{ "object is not callable" };
        }
      }
    }
  }
}
