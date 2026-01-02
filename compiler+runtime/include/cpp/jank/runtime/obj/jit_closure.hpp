#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/callable.hpp>

namespace jank::runtime::obj
{
  using jit_closure_ref = oref<struct jit_closure>;

  struct jit_closure : behavior::callable
  {
    static constexpr object_type obj_type{ object_type::jit_closure };
    static constexpr bool pointer_free{ false };

    jit_closure() = default;
    jit_closure(jit_closure &&) noexcept = default;
    jit_closure(jit_closure const &) = default;
    jit_closure(arity_flag_t arity_flags, void *context);
    jit_closure(object_ref const meta);

    /* behavior::object_like */
    bool equal(object const &) const;
    jtl::immutable_string to_string();
    void to_string(jtl::string_builder &buff);
    jtl::immutable_string to_code_string();
    uhash to_hash() const;

    /* behavior::metadatable */
    jit_closure_ref with_meta(object_ref const m);

    /* behavior::callable */
    object_ref call() final;
    object_ref call(object_ref const) final;
    object_ref call(object_ref const, object_ref const) final;
    object_ref call(object_ref const, object_ref const, object_ref const) final;
    object_ref call(object_ref const, object_ref const, object_ref const, object_ref const) final;
    object_ref call(object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const) final;
    object_ref call(object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const) final;
    object_ref call(object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const) final;
    object_ref call(object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const) final;
    object_ref call(object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const) final;
    object_ref call(object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const) final;

    arity_flag_t get_arity_flags() const final;

    object_ref this_object_ref() final;

    object base{ obj_type };
    void *context{};
    object *(*arity_0)(object *){};
    object *(*arity_1)(object *, object *){};
    object *(*arity_2)(object *, object *, object *){};
    object *(*arity_3)(object *, object *, object *, object *){};
    object *(*arity_4)(object *, object *, object *, object *, object *){};
    object *(*arity_5)(object *, object *, object *, object *, object *, object *){};
    object *(*arity_6)(object *, object *, object *, object *, object *, object *, object *){};
    object *(*arity_7)(object *,

                       object *,
                       object *,
                       object *,
                       object *,
                       object *,
                       object *,
                       object *){};
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
    jtl::option<object_ref> meta;
    arity_flag_t arity_flags{};
  };
}
