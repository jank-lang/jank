#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/callable.hpp>

namespace jank::runtime::obj
{
  using jit_function_ref = oref<struct jit_function>;

  struct jit_function
    : object
    , behavior::callable
  {
    static constexpr object_type obj_type{ object_type::jit_function };
    static constexpr bool pointer_free{ false };

    jit_function();
    jit_function(jit_function &&) noexcept = default;
    jit_function(jit_function const &) = default;
    jit_function(arity_flag_t arity_flags);
    jit_function(object_ref meta);

    /* behavior::object_like */
    bool equal(object const &) const override;
    jtl::immutable_string to_string() const override;
    void to_string(jtl::string_builder &buff) const override;
    jtl::immutable_string to_code_string() const override;
    uhash to_hash() const override;

    /* behavior::metadatable */
    jit_function_ref with_meta(object_ref m);

    /* behavior::callable */
    object_ref call() override;
    object_ref call(object_ref) override;
    object_ref call(object_ref, object_ref) override;
    object_ref call(object_ref, object_ref, object_ref) override;
    object_ref call(object_ref, object_ref, object_ref, object_ref) override;
    object_ref call(object_ref, object_ref, object_ref, object_ref, object_ref) override;
    object_ref
      call(object_ref, object_ref, object_ref, object_ref, object_ref, object_ref) override;
    object_ref
      call(object_ref, object_ref, object_ref, object_ref, object_ref, object_ref, object_ref)
        override;
    object_ref call(object_ref,
                    object_ref,
                    object_ref,
                    object_ref,
                    object_ref,
                    object_ref,
                    object_ref,
                    object_ref) override;
    object_ref call(object_ref,
                    object_ref,
                    object_ref,
                    object_ref,
                    object_ref,
                    object_ref,
                    object_ref,
                    object_ref,
                    object_ref) override;
    object_ref call(object_ref,
                    object_ref,
                    object_ref,
                    object_ref,
                    object_ref,
                    object_ref,
                    object_ref,
                    object_ref,
                    object_ref,
                    object_ref) override;

    arity_flag_t get_arity_flags() const override;
    object_ref this_object_ref() override;

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
