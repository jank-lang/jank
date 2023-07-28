#pragma once

#include <jank/option.hpp>

namespace jank::runtime
{
  using object_ptr = native_box<struct object>;

  namespace obj
  {
    using list = static_object<object_type::list>;
    using list_ptr = native_box<list>;
  }

  constexpr size_t const max_params{ 10 };

  struct variadic_tag
  { };

  /* When analysis can match a call to a fn definition, we can know the
   * supported arities at compile-time, but that's not always the case in
   * dynamic code. We may not every know if the source is callable at all, so
   * codegen will use this suite of fns instead. */
  object_ptr dynamic_call(object_ptr source);
  object_ptr dynamic_call(object_ptr source, object_ptr);
  object_ptr dynamic_call(object_ptr source, object_ptr, object_ptr);
  object_ptr dynamic_call(object_ptr source, object_ptr, object_ptr, object_ptr);
  object_ptr dynamic_call(object_ptr source, object_ptr, object_ptr, object_ptr, object_ptr);
  object_ptr dynamic_call(object_ptr source, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr);
  object_ptr dynamic_call(object_ptr source, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr);
  object_ptr dynamic_call(object_ptr source, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr);
  object_ptr dynamic_call(object_ptr source, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr);
  object_ptr dynamic_call(object_ptr source, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr);
  object_ptr dynamic_call(object_ptr source, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr);
  object_ptr dynamic_call(object_ptr source, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, obj::list_ptr);

  object_ptr apply_to(object_ptr source, object_ptr args);

  namespace behavior
  {
    /* TODO: Is this needed? A non-callable function-like would need to define all call overloads? :( */
    template <typename T>
    concept function_like = requires(T * const t)
    {
      { t->call(object_ptr{}) } -> std::convertible_to<object_ptr>;
      { t->call(object_ptr{}, object_ptr{}) } -> std::convertible_to<object_ptr>;
      { t->call(object_ptr{}, object_ptr{}, object_ptr{}) } -> std::convertible_to<object_ptr>;
      { t->call(object_ptr{}, object_ptr{}, object_ptr{}, object_ptr{}) } -> std::convertible_to<object_ptr>;
      { t->call(object_ptr{}, object_ptr{}, object_ptr{}, object_ptr{}, object_ptr{}) } -> std::convertible_to<object_ptr>;
      { t->call(object_ptr{}, object_ptr{}, object_ptr{}, object_ptr{}, object_ptr{}, object_ptr{}) } -> std::convertible_to<object_ptr>;
      { t->call(object_ptr{}, object_ptr{}, object_ptr{}, object_ptr{}, object_ptr{}, object_ptr{}, object_ptr{}) } -> std::convertible_to<object_ptr>;
      { t->call(object_ptr{}, object_ptr{}, object_ptr{}, object_ptr{}, object_ptr{}, object_ptr{}, object_ptr{}, object_ptr{}) } -> std::convertible_to<object_ptr>;
      { t->call(object_ptr{}, object_ptr{}, object_ptr{}, object_ptr{}, object_ptr{}, object_ptr{}, object_ptr{}, object_ptr{}, object_ptr{}) } -> std::convertible_to<object_ptr>;

      { t->get_variadic_arg_position() } -> std::convertible_to<size_t>;
    };

    struct callable
    {
      virtual ~callable() = default;

      virtual object_ptr call() const;
      virtual object_ptr call(object_ptr) const;
      virtual object_ptr call(object_ptr, object_ptr) const;
      virtual object_ptr call(object_ptr, object_ptr, object_ptr) const;
      virtual object_ptr call(object_ptr, object_ptr, object_ptr, object_ptr) const;
      virtual object_ptr call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr) const;
      virtual object_ptr call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr) const;
      virtual object_ptr call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr) const;
      virtual object_ptr call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr) const;
      virtual object_ptr call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr) const;
      virtual object_ptr call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr) const;

      virtual size_t get_variadic_arg_position() const;
    };
    using callable_ptr = native_box<callable>;
  }
}
