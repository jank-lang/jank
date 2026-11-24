#include <jank/compiler_native.hpp>
#include <jank/runtime/convert/function.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/munge.hpp>
#include <jank/runtime/obj/nil.hpp>
#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/obj/persistent_hash_map.hpp>
#include <jank/runtime/obj/keyword.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/analyze/pass/optimize.hpp>
#include <jank/ir/processor.hpp>
#include <jank/codegen/cpp_processor.hpp>
#include <jank/evaluate.hpp>
#include <jank/util/clang_format.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::compiler_native
{
  using namespace jank;
  using namespace jank::runtime;

  object_ref native_source(object_ref const form)
  {
    /* We use a clean analyze::processor so we don't share lifted items from other REPL
     * evaluations. */
    analyze::processor an_prc;
    auto const expr(analyze::pass::optimize(
      an_prc.analyze(form, analyze::expression_position::value).expect_ok()));
    auto const wrapped_expr(evaluate::wrap_expression(expr, "native_source"));
    auto const &module(
      expect_object<runtime::ns>(__rt_ctx->intern_var("clojure.core", "*ns*").expect_ok()->deref())
        ->to_string());
    auto const ir_mod{ ir::create(wrapped_expr, module, codegen::compilation_target::eval) };
    auto const generated{ codegen::gen_cpp(ir_mod) };

    return make_box(util::format_cpp_source(generated.declaration).expect_ok());
  }
}
