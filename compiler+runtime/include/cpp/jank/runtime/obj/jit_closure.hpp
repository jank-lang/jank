#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/callable.hpp>

namespace jank::runtime
{
  template <>
  struct static_object<object_type::jit_closure>
    : gc
    , behavior::callable
  {
    static constexpr native_bool pointer_free{ false };

    static_object() = default;
    static_object(static_object &&) = default;
    static_object(static_object const &) = default;
    static_object(arity_flag_t arity_flags);
    static_object(arity_flag_t arity_flags, void *context);
    static_object(object_ptr meta);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string();
    void to_string(fmt::memory_buffer &buff);
    native_persistent_string to_code_string();
    native_hash to_hash() const;

    /* behavior::metadatable */
    native_box<static_object> with_meta(object_ptr m);

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

  namespace obj
  {
    using jit_closure = static_object<object_type::jit_closure>;
    using jit_closure_ptr = native_box<jit_closure>;
  }
}
