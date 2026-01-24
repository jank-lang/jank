#pragma once

#include <mutex>

#include <jank/runtime/object.hpp>
#include <jank/runtime/behavior/callable.hpp>

namespace jank::runtime
{
  struct var;
}

namespace jank::runtime::obj
{
  using deferred_cpp_function_ref = oref<struct deferred_cpp_function>;
  using jit_function_ref = oref<struct jit_function>;
  using var_ref = oref<runtime::var>;

  /* When compiling with C++ codegen, we support lazily compiling functions using this
   * deferred function object. When a function is within a def, we will create this function
   * type instead of JIT compiling the C++ to create an actual function object. This function
   * object just stores the generated C++ code and will JIT compile it if the function gets
   * called.
   *
   * In many cases, especially when compiling modules, most functions are not called. This
   * allows us to drastically cut down on compile times, since JIT compiling C++ is a very
   * expensive task.
   *
   * When this function is called, it will JIT compile the C++ code, get a real function
   * object, replace the root of the var with that object, and then continue to proxy
   * calls to that object for anyone who still has a handle to this one. */
  struct deferred_cpp_function
    : object
    , behavior::callable
  {
    static constexpr object_type obj_type{ object_type::deferred_cpp_function };
    static constexpr bool pointer_free{ false };

    deferred_cpp_function(object_ref const meta,
                          jtl::immutable_string const &declaration_code,
                          jtl::immutable_string const &expression_code,
                          var_ref const var);

    /* behavior::object_like */
    using object::to_string;
    void to_string(jtl::string_builder &buff) const override;

    /* behavior::callable */
    using behavior::callable::call;
    object_ref call(object_ref const) override;

    arity_flag_t get_arity_flags() const override;
    object_ref this_object_ref() override;

    /*** XXX: Everything here is immutable after initialization. ***/
    jtl::option<object_ref> meta;
    var_ref var;

    /*** XXX: Everything here is thread-safe. ***/
    std::recursive_mutex compilation_mutex;
    jit_function_ref compiled_fn;
    jtl::immutable_string declaration_code;
    jtl::immutable_string expression_code;
  };
}
