#include <clang/Interpreter/Value.h>

#include <jank/runtime/obj/deferred_cpp_function.hpp>
#include <jank/runtime/obj/jit_function.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/obj/keyword.hpp>
#include <jank/runtime/var.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/core/call.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::runtime::obj
{
  deferred_cpp_function::deferred_cpp_function(object_ref const meta,
                                               jtl::immutable_string const &declaration_code,
                                               jtl::immutable_string const &expression_code,
                                               var_ref const var)
    : object{ obj_type, obj_behaviors }
    , meta{ meta }
    , var{ var }
    , declaration_code{ declaration_code }
    , expression_code{ expression_code }
  {
  }

  void deferred_cpp_function::to_string(jtl::string_builder &buff) const
  {
    auto const name(meta->get(__rt_ctx->intern_keyword("name").expect_ok()));
    util::format_to(
      buff,
      "#object [{} {} {}]",
      (name->type == object_type::nil ? "unknown" : try_object<persistent_string>(name)->data),
      object_type_str(type),
      this);
  }

  object_ref deferred_cpp_function::call(object_ref const args) const
  {
    std::lock_guard<std::recursive_mutex> const lock{ compilation_mutex };
    /* It's possible that we're called again, even after we've compiled our actual function.
     * This can happen if the value of this function is captured, rather than used directly
     * through a var. In that case, we just proxy the args on to the compiled fn. */
    if(compiled_fn.is_some())
    {
      return apply_to(compiled_fn, args);
    }

    // auto const name(meta->get(__rt_ctx->intern_keyword("name").expect_ok()));
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

  callable_arity_flags deferred_cpp_function::get_arity_flags() const
  {
    /* Deferred fns are always [& args], which they then apply to the proxied fn. */
    return build_arity_flags(0, true, false);
  }
}
