#pragma once

#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using persistent_list_ref = oref<struct persistent_list>;
  }

  /* jank has a max number of fixed parameters. Beyond this, variadics must be used.
   * Clojure uses something like 15, instead of our 10, but I really don't think that's
   * needed and it ends up requiring a lot of code to implement. Until we have a good
   * reason to bump this, I think we're good. */
  constexpr usize const max_params{ 10 };

  /* When analysis can match a call to a fn definition, we can know the
   * supported arities at compile-time, but that's not always the case in
   * dynamic code. We may not every know if the source is callable at all, so
   * codegen will use this suite of fns instead. */
  object_ref dynamic_call(object_ref const source);
  object_ref dynamic_call(object_ref const source, object_ref const);
  object_ref dynamic_call(object_ref const source, object_ref const, object_ref const);
  object_ref
  dynamic_call(object_ref const source, object_ref const, object_ref const, object_ref const);
  object_ref dynamic_call(object_ref const source,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const);
  object_ref dynamic_call(object_ref const source,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const);
  object_ref dynamic_call(object_ref const source,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const);
  object_ref dynamic_call(object_ref const source,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const);
  object_ref dynamic_call(object_ref const source,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const);
  object_ref dynamic_call(object_ref const source,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const);
  object_ref dynamic_call(object_ref const source,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const);
  object_ref dynamic_call(object_ref const source,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          object_ref const,
                          obj::persistent_list_ref const);

  object_ref apply_to(object_ref const source, object_ref const args);

  static constexpr callable_arity_flags mask_variadic_arity(u8 const pos)
  {
    return (0b10000000 | pos);
  }

  static constexpr callable_arity_flags
  extract_variadic_arity_mask(callable_arity_flags const arity_flags)
  {
    return (arity_flags & 0b10001111);
  }

  static constexpr bool is_variadic_ambiguous(callable_arity_flags const arity_flags)
  {
    return (arity_flags & 0b01000000);
  }

  static constexpr callable_arity_flags build_arity_flags(u8 const highest_fixed_arity,
                                                          bool const is_variadic,
                                                          bool const is_variadic_ambiguous)
  {
    return (is_variadic << 7) | (is_variadic_ambiguous << 6) | highest_fixed_arity;
  }
}
