#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/callable.hpp>

namespace jank::runtime::obj
{
  using jit_function_ref = jtl::object_ref<struct jit_function>;

  struct jit_function
    : gc
    , behavior::callable
  {
    static constexpr object_type obj_type{ object_type::jit_function };
    static constexpr native_bool pointer_free{ false };

    jit_function() = default;
    jit_function(jit_function &&) noexcept = default;
    jit_function(jit_function const &) = default;
    jit_function(arity_flag_t arity_flags);
    jit_function(object_ptr meta);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    jtl::immutable_string to_string();
    void to_string(util::string_builder &buff);
    jtl::immutable_string to_code_string();
    native_hash to_hash() const;

    /* behavior::metadatable */
    jit_function_ref with_meta(object_ptr m);

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
    jtl::option<object_ptr> meta;
    arity_flag_t arity_flags{};
  };
}
