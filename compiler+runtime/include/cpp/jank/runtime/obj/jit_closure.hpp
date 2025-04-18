#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/callable.hpp>

namespace jank::runtime::obj
{
  using jit_closure_ref = oref<struct jit_closure>;

  struct jit_closure
    : gc
    , behavior::callable
  {
    static constexpr object_type obj_type{ object_type::jit_closure };
    static constexpr bool pointer_free{ false };

    jit_closure() = default;
    jit_closure(jit_closure &&) noexcept = default;
    jit_closure(jit_closure const &) = default;
    jit_closure(arity_flag_t arity_flags, void *context);
    jit_closure(object_ref meta);

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string();
    void to_string(util::string_builder &buff);
    jtl::immutable_string to_code_string();
    native_hash to_hash() const;

    /* behavior::metadatable */
    jit_closure_ref with_meta(object_ref m);

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

    arity_flag_t get_arity_flags() const final;

    object_ref this_object_ref() final;

    object base{ obj_type };
    void *context{};
    object *(*arity_0)(void *){};
    object *(*arity_1)(void *, object *){};
    object *(*arity_2)(void *, object *, object *){};
    object *(*arity_3)(void *, object *, object *, object *){};
    object *(*arity_4)(void *, object *, object *, object *, object *){};
    object *(*arity_5)(void *, object *, object *, object *, object *, object *){};
    object *(*arity_6)(void *, object *, object *, object *, object *, object *, object *){};
    object *(
      *arity_7)(void *, object *, object *, object *, object *, object *, object *, object *){};
    object *(*arity_8)(void *,
                       object *,
                       object *,
                       object *,
                       object *,
                       object *,
                       object *,
                       object *,
                       object *){};
    object *(*arity_9)(void *,
                       object *,
                       object *,
                       object *,
                       object *,
                       object *,
                       object *,
                       object *,
                       object *,
                       object *){};
    object *(*arity_10)(void *,
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
    jtl::option<object_ref> meta;
    arity_flag_t arity_flags{};
  };
}
