#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/lazy_meta.hpp>

namespace jank::runtime::obj
{
  using jit_variadic_function_ref = oref<struct jit_variadic_function>;

  struct jit_variadic_function : object
  {
    static constexpr object_type obj_type{ object_type::jit_variadic_function };
    static constexpr object_behavior obj_behaviors{ object_behavior::call };
    static constexpr bool pointer_free{ false };

    jit_variadic_function();
    jit_variadic_function(jit_variadic_function &&) noexcept = default;
    jit_variadic_function(jit_variadic_function const &) = default;
    jit_variadic_function(callable_arity_flags arity_flags);
    jit_variadic_function(object_ref const meta);

    /* behavior::object_like */
    using object::to_string;
    void to_string(jtl::string_builder &buff) const override;

    /* behavior::metadatable */
    jit_variadic_function_ref with_meta(object_ref const m);
    object_ref get_meta() const;
    void set_meta(object_ref const o);

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
    object_ref call(object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const,
                    object_ref const) const override;

    /*** XXX: Everything here is immutable after initialization. ***/
    object_ref (*arity_0)(object_ref){};
    object_ref (*arity_1)(object_ref, object_ref){};
    object_ref (*arity_2)(object_ref, object_ref, object_ref){};
    object_ref (*arity_3)(object_ref, object_ref, object_ref, object_ref){};
    object_ref (*arity_4)(object_ref, object_ref, object_ref, object_ref, object_ref){};
    object_ref (*arity_5)(object_ref, object_ref, object_ref, object_ref, object_ref, object_ref){};
    object_ref (*arity_6)(object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref){};
    object_ref (*arity_7)(object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref){};
    object_ref (*arity_8)(object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref){};
    object_ref (*arity_9)(object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref,
                          object_ref){};
    object_ref (*arity_10)(object_ref,
                           object_ref,
                           object_ref,
                           object_ref,
                           object_ref,
                           object_ref,
                           object_ref,
                           object_ref,
                           object_ref,
                           object_ref,
                           object_ref){};
    callable_arity_flags arity_flags{};

    /*** XXX: Everything here is thead-safe. ***/
  private:
    lazy_meta meta;
  };
}
