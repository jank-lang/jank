#include <jank/compiler_native.hpp>
#include <jank/runtime/convert/function.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/munge.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/obj/persistent_hash_map.hpp>
#include <jank/runtime/obj/keyword.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/evaluate.hpp>
#include <jank/codegen/llvm_processor.hpp>

namespace jank::compiler_native
{
  using namespace jank;
  using namespace jank::runtime;

  static object_ptr native_source(object_ptr const form)
  {
    /* We use a clean analyze::processor so we don't share lifted items from other REPL
     * evaluations. */
    analyze::processor an_prc{ *__rt_ctx };
    auto const expr(an_prc.analyze(form, analyze::expression_position::value).expect_ok());
    auto const wrapped_expr(evaluate::wrap_expression(expr, "native_source", {}));
    auto const &module(
      expect_object<runtime::ns>(__rt_ctx->intern_var("clojure.core", "*ns*").expect_ok()->deref())
        ->to_string());

    codegen::llvm_processor cg_prc{ wrapped_expr, module, codegen::compilation_target::eval };
    cg_prc.gen().expect_ok();

    /* TODO: Return a string, don't print it. */
    cg_prc.ctx->module->print(llvm::outs(), nullptr);
    return obj::nil::nil_const();
  }
}

jank_object_ptr jank_load_jank_compiler_native()
{
  using namespace jank;
  using namespace jank::runtime;

  auto const ns(__rt_ctx->intern_ns("jank.compiler-native"));

  auto const intern_fn([=](jtl::immutable_string const &name, auto const fn) {
    ns->intern_var(name)->bind_root(
      make_box<obj::native_function_wrapper>(convert_function(fn))
        ->with_meta(obj::persistent_hash_map::create_unique(std::make_pair(
          __rt_ctx->intern_keyword("name").expect_ok(),
          make_box(obj::symbol{ __rt_ctx->current_ns()->to_string(), name }.to_string())))));
  });
  intern_fn("native-source", &compiler_native::native_source);

  return erase(obj::nil::nil_const());
}
