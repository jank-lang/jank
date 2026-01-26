#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using jit_closure_ref = oref<struct jit_closure>;

  struct jit_closure : object
  {
    static constexpr object_type obj_type{ object_type::jit_closure };
    static constexpr object_behavior obj_behaviors{ object_behavior::call };
    static constexpr bool pointer_free{ false };

    jit_closure();
    jit_closure(jit_closure &&) noexcept = default;
    jit_closure(jit_closure const &) = default;
    jit_closure(callable_arity_flags arity_flags, void *context);
    jit_closure(object_ref const meta);

    /* behavior::object_like */
    using object::to_string;
    void to_string(jtl::string_builder &buff) const override;

    /* behavior::metadatable */
    jit_closure_ref with_meta(object_ref const m);
    object_ref get_meta() const;

    /* behavior::callable */
    object_ref call() const final;
    object_ref call(object_ref const) const final;
    object_ref call(object_ref const, object_ref const) const final;
    object_ref call(object_ref const, object_ref const, object_ref const) const final;
    object_ref
    call(object_ref const, object_ref const, object_ref const, object_ref const) const final;
    object_ref call(object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const) const final;
    object_ref call(object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const) const final;
    object_ref call(object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const) const final;
    object_ref call(object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const) const final;
    object_ref call(object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const) const final;
    object_ref call(object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const) const final;

    callable_arity_flags get_arity_flags() const final;

    /*** XXX: Everything here is immutable after initialization. ***/
    void *context{};
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
