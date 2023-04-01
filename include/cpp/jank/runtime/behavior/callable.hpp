#pragma once

#include <jank/option.hpp>

namespace jank::runtime
{
  using object_ptr = struct object*;

  namespace obj
  { using list_ptr = struct list*; }

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
    struct callable
    {
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

      virtual option<size_t> get_variadic_arg_position() const;
    };
    using callable_ptr = native_box<callable>;
  }
}
