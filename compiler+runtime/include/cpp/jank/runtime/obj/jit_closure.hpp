#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/callable.hpp>

namespace jank::runtime::obj
{
  using jit_closure_ptr = native_box<struct jit_closure>;

  struct jit_closure
    : gc
    , behavior::callable
  {
    static constexpr object_type obj_type{ object_type::jit_closure };
    static constexpr native_bool pointer_free{ false };

    jit_closure() = default;
    jit_closure(jit_closure &&) noexcept = default;
    jit_closure(jit_closure const &) = default;
    jit_closure(arity_flag_t arity_flags, void *context);
    jit_closure(object_ptr meta);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string();
    void to_string(fmt::memory_buffer &buff);
    native_persistent_string to_code_string();
    native_hash to_hash() const;

    /* behavior::metadatable */
    jit_closure_ptr with_meta(object_ptr m);

    /* behavior::callable */
    object_ptr call() final;
    object_ptr call(object_ptr) final;
    object_ptr call(object_ptr, object_ptr) final;
    object_ptr call(object_ptr, object_ptr, object_ptr) final;
    object_ptr call(object_ptr, object_ptr, object_ptr, object_ptr) final;
    object_ptr call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr) final;
    object_ptr call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr) final;
    object_ptr
      call(object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr, object_ptr)
        final;
    object_ptr call(object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr) final;
    object_ptr call(object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr) final;
    object_ptr call(object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr,
                    object_ptr) final;

    arity_flag_t get_arity_flags() const final;

    object_ptr this_object_ptr() final;

    object base{ object_type::jit_closure };
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
    option<object_ptr> meta;
    arity_flag_t arity_flags{};
  };
}
