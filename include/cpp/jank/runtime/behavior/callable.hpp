#pragma once

#include <jank/runtime/memory_pool.hpp>
#include <jank/runtime/detail/type.hpp>

namespace jank::runtime
{
  using object_ptr = detail::box_type<struct object>;

  struct variadic_tag
  { };

  /* When analysis can match a call to a fn definition, we can know the
   * supported arities at compile-time, but that's not always the case in
   * dynamic code. We may not every know if the source is callable at all, so
   * codegen will use this suite of fns instead. */
  object_ptr dynamic_call(object_ptr const &source);
  object_ptr dynamic_call(object_ptr const &source, object_ptr const&);
  object_ptr dynamic_call(object_ptr const &source, object_ptr const&, object_ptr const&);
  object_ptr dynamic_call(object_ptr const &source, object_ptr const&, object_ptr const&, object_ptr const&);
  object_ptr dynamic_call(object_ptr const &source, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&);
  object_ptr dynamic_call(object_ptr const &source, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&);
  object_ptr dynamic_call(object_ptr const &source, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&);
  object_ptr dynamic_call(object_ptr const &source, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&);
  object_ptr dynamic_call(object_ptr const &source, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&);
  object_ptr dynamic_call(object_ptr const &source, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&);
  object_ptr dynamic_call(object_ptr const &source, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&);

  namespace behavior
  {
    struct callable : virtual pool_item_common_base
    {
      virtual object_ptr call() const;
      virtual object_ptr call(object_ptr const&) const;
      virtual object_ptr call(object_ptr const&, object_ptr const&) const;
      virtual object_ptr call(object_ptr const&, object_ptr const&, object_ptr const&) const;
      virtual object_ptr call(object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&) const;
      virtual object_ptr call(object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&) const;
      virtual object_ptr call(object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&) const;
      virtual object_ptr call(object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&) const;
      virtual object_ptr call(object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&) const;
      virtual object_ptr call(object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&) const;
      virtual object_ptr call(object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&) const;

      /* Returns the number of fixed params before the 'rest' args, for
       * variadic arities. If this callable doesn't have a variadic arity, this
       * should return none. The name here comes from Clojure's compiler. */
      virtual option<size_t> get_required_arity() const;
    };
    using callable_ptr = detail::box_type<callable>;
  }
}
