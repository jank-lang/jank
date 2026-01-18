#include <clang/Interpreter/Value.h>

#include <jank/runtime/obj/deferred_cpp_function.hpp>
#include <jank/runtime/obj/jit_function.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/obj/keyword.hpp>
#include <jank/runtime/var.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/core.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::runtime::obj
{
  deferred_cpp_function::deferred_cpp_function(object_ref const meta,
                                               jtl::immutable_string const &declaration_code,
                                               jtl::immutable_string const &expression_code,
                                               var_ref const var)
    : meta{ meta }
    , declaration_code{ declaration_code }
    , expression_code{ expression_code }
    , var{ var }
  {
  }

  bool deferred_cpp_function::equal(object const &rhs) const
  {
    return &base == &rhs;
  }

  jtl::immutable_string deferred_cpp_function::to_string()
  {
    jtl::string_builder buff;
    to_string(buff);
    return buff.release();
  }

  void deferred_cpp_function::to_string(jtl::string_builder &buff)
  {
    auto const name(get(meta.unwrap_or(jank_nil()), __rt_ctx->intern_keyword("name").expect_ok()));
    util::format_to(
      buff,
      "#object [{} {} {}]",
      (name->type == object_type::nil ? "unknown" : try_object<persistent_string>(name)->data),
      object_type_str(base.type),
      &base);
  }

  jtl::immutable_string deferred_cpp_function::to_code_string()
  {
    return to_string();
  }

  uhash deferred_cpp_function::to_hash() const
  {
    return static_cast<uhash>(reinterpret_cast<uintptr_t>(this));
  }

  deferred_cpp_function_ref deferred_cpp_function::with_meta(object_ref const m)
  {
    auto const new_meta(behavior::detail::validate_meta(m));
    meta = new_meta;
    return this;
  }

  object_ref deferred_cpp_function::call(object_ref const args)
  {
    /* It's possible that we're called again, even after we've compiled our actual function.
     * This can happen if the value of this function is captured, rather than used directly
     * through a var. In that case, we just proxy the args on to the compiled fn. */
    if(compiled_fn.is_some())
    {
      return apply_to(compiled_fn, args);
    }

    //auto const name(get(meta.unwrap_or(jank_nil()), __rt_ctx->intern_keyword("name").expect_ok()));
    //util::println(
    //  "lazily creating {}",
    //  (name->type == object_type::nil ? "unknown" : try_object<persistent_string>(name)->data));

    /* On the first invocation, we don't have a compiled_fn. We compile our C++ code, get a fn,
     * rebind the root of the var, and then apply our args to the new fn.*/
    __rt_ctx->jit_prc.eval_string(declaration_code);
    clang::Value v;
    __rt_ctx->jit_prc.eval_string({ expression_code.data(), expression_code.size() }, &v);
    compiled_fn = try_object<obj::jit_function>(v.convertTo<runtime::object *>());
    compiled_fn->meta = meta;
    var->bind_root(compiled_fn);

    /* Clear these just to free up some memory. */
    declaration_code = "";
    expression_code = "";

    return apply_to(compiled_fn, args);
  }

  behavior::callable::arity_flag_t deferred_cpp_function::get_arity_flags() const
  {
    /* Deferred fns are always [& args], which they then apply to the proxied fn. */
    return behavior::callable::build_arity_flags(0, true, false);
  }

  object_ref deferred_cpp_function::this_object_ref()
  {
    return &this->base;
  }
}
