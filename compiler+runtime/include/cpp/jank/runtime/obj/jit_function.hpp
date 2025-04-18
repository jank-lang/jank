#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/callable.hpp>

namespace jank::runtime::obj
{
  using jit_function_ref = oref<struct jit_function>;

  struct jit_function
    : gc
    , behavior::callable
  {
    static constexpr object_type obj_type{ object_type::jit_function };
    static constexpr bool pointer_free{ false };

    jit_function() = default;
    jit_function(jit_function &&) noexcept = default;
    jit_function(jit_function const &) = default;
    jit_function(arity_flag_t arity_flags);
    jit_function(object_ref meta);

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string();
    void to_string(util::string_builder &buff);
    jtl::immutable_string to_code_string();
    native_hash to_hash() const;

    /* behavior::metadatable */
    jit_function_ref with_meta(object_ref m);

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
    object *(*arity_0)(){};
    object *(*arity_1)(object *){};
    object *(*arity_2)(object *, object *){};
    object *(*arity_3)(object *, object *, object *){};
    object *(*arity_4)(object *, object *, object *, object *){};
    object *(*arity_5)(object *, object *, object *, object *, object *){};
    object *(*arity_6)(object *, object *, object *, object *, object *, object *){};
    object *(*arity_7)(object *, object *, object *, object *, object *, object *, object *){};
    object *(
      *arity_8)(object *, object *, object *, object *, object *, object *, object *, object *){};
    object *(*arity_9)(object *,
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
                        object *){};
    jtl::option<object_ref> meta;
    arity_flag_t arity_flags{};
  };
}
