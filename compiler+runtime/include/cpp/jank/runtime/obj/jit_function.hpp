#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using jit_function_ref = oref<struct jit_function>;

  struct jit_function : object
  {
    static constexpr object_type obj_type{ object_type::jit_function };
    static constexpr object_behavior obj_behaviors{ object_behavior::call };
    static constexpr bool pointer_free{ false };

    jit_function();
    jit_function(jit_function &&) noexcept = default;
    jit_function(jit_function const &) = default;
    jit_function(callable_arity_flags arity_flags);
    jit_function(object_ref const meta);

    /* behavior::object_like */
    using object::to_string;
    void to_string(jtl::string_builder &buff) const override;

    /* behavior::metadatable */
    jit_function_ref with_meta(object_ref const m);
    object_ref get_meta() const;

    /* behavior::callable */
    object_ref call() const override;
    object_ref call(object_ref const) const override;
    object_ref call(object_ref const, object_ref const) const override;
    object_ref call(object_ref const, object_ref const, object_ref const) const override;
    object_ref
    call(object_ref const, object_ref const, object_ref const, object_ref const) const override;
    object_ref call(object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const) const override;
    object_ref call(object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const) const override;
    object_ref call(object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const) const override;
    object_ref call(object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const) const override;
    object_ref call(object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const) const override;
    object_ref call(object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const) const override;

    callable_arity_flags get_arity_flags() const override;

    /*** XXX: Everything here is immutable after initialization. ***/
    object *(*arity_0)(object *){};
    object *(*arity_1)(object *, object *){};
    object *(*arity_2)(object *, object *, object *){};
    object *(*arity_3)(object *, object *, object *, object *){};
    object *(*arity_4)(object *, object *, object *, object *, object *){};
    object *(*arity_5)(object *, object *, object *, object *, object *, object *){};
    object *(*arity_6)(object *, object *, object *, object *, object *, object *, object *){};
    object *(
      *arity_7)(object *, object *, object *, object *, object *, object *, object *, object *){};
    object *(*arity_8)(object *,
                       object *,
                       object *,
                       object *,
                       object *,
                       object *,
                       object *,
                       object *,
                       object *){};
    object *(*arity_9)(object *,
                       object *,
                       object *,
                       object *,
                       object *,
                       object *,
                       object *,
                       object *,
                       object *,
                       object *){};
    object *(*arity_10)(object *,
                        object *,
                        object *,
                        object *,
                        object *,
                        object *,
                        object *,
                        object *,
                        object *,
                        object *,
                        object *){};
    object_ref meta;
    callable_arity_flags arity_flags{};
  };
}
