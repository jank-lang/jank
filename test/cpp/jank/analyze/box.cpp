#include <jank/runtime/context.hpp>
#include <jank/jit/processor.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

/* TODO: Add a semantic pass to detect unboxed locals with no unboxed usages; box them. */
/* TODO: Test for forcing boxing with recur. */
/* TODO: Optimize these by reducing prelude redundancy. */

namespace jank::analyze
{
  using runtime::detail::equal;

  TEST_CASE("Unboxed local")
  {
    runtime::context rt_ctx;

    SUBCASE("No boxed usage")
    {
      auto const res(rt_ctx.analyze_string("(let* [a 1 b a])"));
      CHECK_EQ(res.size(), 1);

      auto const map(res[0]->to_runtime_data());
      auto const a_binding(runtime::get_in(map, rt_ctx.eval_string(R"(["pairs" 0 0])")));

      CHECK(equal(runtime::get(a_binding, make_box("needs_box")), make_box(false)));
      CHECK(equal(runtime::get(a_binding, make_box("has_boxed_usage")), make_box(false)));
      CHECK(equal(runtime::get(a_binding, make_box("has_unboxed_usage")), make_box(true)));
    }

    SUBCASE("Unboxed arithmetic")
    {
      runtime::context rt_ctx;
      rt_ctx.load_module("/clojure.core");

      auto const res(rt_ctx.analyze_string("(let* [a 1 b (* a 2.0) c (/ b 2.0)])"));
      CHECK_EQ(res.size(), 1);

      auto const map(res[0]->to_runtime_data());

      auto const b_binding(runtime::get_in(map, rt_ctx.eval_string(R"(["pairs" 1 0])")));
      CHECK(equal(runtime::get(b_binding, make_box("needs_box")), make_box(false)));
      CHECK(equal(runtime::get(b_binding, make_box("has_boxed_usage")), make_box(false)));
      CHECK(equal(runtime::get(b_binding, make_box("has_unboxed_usage")), make_box(true)));

      auto const c_binding(runtime::get_in(map, rt_ctx.eval_string(R"(["pairs" 2 0])")));
      CHECK(equal(runtime::get(c_binding, make_box("needs_box")), make_box(false)));
      CHECK(equal(runtime::get(c_binding, make_box("has_boxed_usage")), make_box(false)));
      CHECK(equal(runtime::get(c_binding, make_box("has_unboxed_usage")), make_box(false)));
    }

    SUBCASE("Boxed usage, directly")
    {
      auto const res(rt_ctx.analyze_string("(let* [a 1 b a] a)"));
      CHECK_EQ(res.size(), 1);

      auto const map(res[0]->to_runtime_data());
      auto const a_binding(runtime::get_in(map, rt_ctx.eval_string(R"(["pairs" 0 0])")));

      CHECK(equal(runtime::get(a_binding, make_box("needs_box")), make_box(false)));
      CHECK(equal(runtime::get(a_binding, make_box("has_boxed_usage")), make_box(true)));
      CHECK(equal(runtime::get(a_binding, make_box("has_unboxed_usage")), make_box(true)));
    }

    SUBCASE("Boxed usage, indirectly")
    {
      auto const res(rt_ctx.analyze_string("(let* [a 1 b a] b)"));
      CHECK_EQ(res.size(), 1);

      auto const map(res[0]->to_runtime_data());
      auto const a_binding(runtime::get_in(map, rt_ctx.eval_string(R"(["pairs" 0 0])")));
      auto const b_binding(runtime::get_in(map, rt_ctx.eval_string(R"(["pairs" 0 1])")));

      CHECK(equal(runtime::get(a_binding, make_box("needs_box")), make_box(false)));
      CHECK(equal(runtime::get(a_binding, make_box("has_boxed_usage")), make_box(true)));
      CHECK(equal(runtime::get(a_binding, make_box("has_unboxed_usage")), make_box(false)));

      CHECK(equal(runtime::get(b_binding, make_box("needs_box")), make_box(false)));
      CHECK(equal(runtime::get(b_binding, make_box("has_boxed_usage")), make_box(true)));
      CHECK(equal(runtime::get(b_binding, make_box("has_unboxed_usage")), make_box(false)));
    }

    SUBCASE("Captured, no unboxed usage")
    {
      auto const res(rt_ctx.analyze_string("(let* [a 1 f (fn* [] a)] (f))", false));
      CHECK_EQ(res.size(), 1);

      auto const map(res[0]->to_runtime_data());

      auto const a_binding(runtime::get_in(map, rt_ctx.eval_string(R"(["pairs" 0 0])")));
      CHECK(equal(runtime::get(a_binding, make_box("needs_box")), make_box(false)));
      CHECK(equal(runtime::get(a_binding, make_box("has_boxed_usage")), make_box(true)));
      CHECK(equal(runtime::get(a_binding, make_box("has_unboxed_usage")), make_box(false)));

      auto const f_fn(runtime::get_in(map, rt_ctx.eval_string(R"(["pairs" 1 1])")));
      auto const captured_a_binding(runtime::get_in(f_fn, rt_ctx.eval_string(R"(["arities" 0 "body" "body" 0 "binding"])")));
      CHECK(equal(runtime::get(captured_a_binding, make_box("needs_box")), make_box(true)));
      CHECK(equal(runtime::get(captured_a_binding, make_box("has_boxed_usage")), make_box(true)));
      CHECK(equal(runtime::get(captured_a_binding, make_box("has_unboxed_usage")), make_box(false)));
    }

    SUBCASE("Captured, unboxed arithmetic usage")
    {
      runtime::context rt_ctx;
      rt_ctx.load_module("/clojure.core");

      auto const res(rt_ctx.analyze_string("(let* [a 1 b (+ a 1.0) f (fn* [] a)] (f))"));
      CHECK_EQ(res.size(), 1);

      auto const map(res[0]->to_runtime_data());

      auto const a_binding(runtime::get_in(map, rt_ctx.eval_string(R"(["pairs" 0 0])")));
      CHECK(equal(runtime::get(a_binding, make_box("needs_box")), make_box(false)));
      CHECK(equal(runtime::get(a_binding, make_box("has_boxed_usage")), make_box(true)));
      CHECK(equal(runtime::get(a_binding, make_box("has_unboxed_usage")), make_box(true)));

      auto const f_fn(runtime::get_in(map, rt_ctx.eval_string(R"(["pairs" 2 1])")));
      auto const captured_a_binding(runtime::get_in(f_fn, rt_ctx.eval_string(R"(["arities" 0 "body" "body" 0 "binding"])")));
      CHECK(equal(runtime::get(captured_a_binding, make_box("needs_box")), make_box(true)));
      CHECK(equal(runtime::get(captured_a_binding, make_box("has_boxed_usage")), make_box(true)));
      CHECK(equal(runtime::get(captured_a_binding, make_box("has_unboxed_usage")), make_box(false)));
    }

    SUBCASE("Captured, box usage indirectly")
    {
      auto const res(rt_ctx.analyze_string("(let* [a 1 b a] (fn* [] b))"));
      CHECK_EQ(res.size(), 1);

      auto const map(res[0]->to_runtime_data());
      auto const a_binding(runtime::get_in(map, rt_ctx.eval_string(R"(["pairs" 0 0])")));
      auto const b_binding(runtime::get_in(map, rt_ctx.eval_string(R"(["pairs" 0 1])")));

      CHECK(equal(runtime::get(a_binding, make_box("needs_box")), make_box(false)));
      CHECK(equal(runtime::get(a_binding, make_box("has_boxed_usage")), make_box(true)));
      CHECK(equal(runtime::get(a_binding, make_box("has_unboxed_usage")), make_box(false)));

      CHECK(equal(runtime::get(b_binding, make_box("needs_box")), make_box(false)));
      CHECK(equal(runtime::get(b_binding, make_box("has_boxed_usage")), make_box(true)));
      CHECK(equal(runtime::get(b_binding, make_box("has_unboxed_usage")), make_box(false)));
    }

    SUBCASE("Sub-expression of a boxed call which doesn't require boxed inputs")
    {
      runtime::context rt_ctx;
      rt_ctx.load_module("/clojure.core");

      auto const res
      (
        rt_ctx.analyze_string
        (
          R"(
          (let* [r 0.0
                 r2 (* r r)]
            (* (+ r2 (- 1.0 r2))
               (pow (- 1.0 r) 5.0)))
          )"
        )
      );
      CHECK_EQ(res.size(), 1);

      auto const map(res[0]->to_runtime_data());

      auto const r_binding(runtime::get_in(map, rt_ctx.eval_string(R"(["pairs" 0 0])")));
      CHECK(equal(runtime::get(r_binding, make_box("needs_box")), make_box(false)));
      CHECK(equal(runtime::get(r_binding, make_box("has_boxed_usage")), make_box(false)));
      CHECK(equal(runtime::get(r_binding, make_box("has_unboxed_usage")), make_box(true)));

      auto const r2_binding(runtime::get_in(map, rt_ctx.eval_string(R"(["pairs" 1 0])")));
      CHECK(equal(runtime::get(r2_binding, make_box("needs_box")), make_box(false)));
      CHECK(equal(runtime::get(r2_binding, make_box("has_boxed_usage")), make_box(false)));
      CHECK(equal(runtime::get(r2_binding, make_box("has_unboxed_usage")), make_box(true)));
    }

    SUBCASE("Captured through multiple levels")
    {
      auto const res
      (
        rt_ctx.analyze_string
        (
          R"(
          (let* [a 10]
            (fn* []
              (fn* []
                a)))
          )"
        )
      );
      CHECK_EQ(res.size(), 1);

      auto const map(res[0]->to_runtime_data());

      auto const a_binding(runtime::get_in(map, rt_ctx.eval_string(R"(["pairs" 0 0])")));
      CHECK(equal(runtime::get(a_binding, make_box("needs_box")), make_box(false)));
      CHECK(equal(runtime::get(a_binding, make_box("has_boxed_usage")), make_box(true)));
      CHECK(equal(runtime::get(a_binding, make_box("has_unboxed_usage")), make_box(false)));

      auto const first_fn(runtime::get_in(map, rt_ctx.eval_string(R"(["body" "body" 0])")));
      auto const second_fn(runtime::get_in(first_fn, rt_ctx.eval_string(R"(["arities" 0 "body" "body" 0])")));
      auto const captured_a_binding(runtime::get_in(second_fn, rt_ctx.eval_string(R"(["arities" 0 "body" "body" 0 "binding"])")));
      CHECK(equal(runtime::get(captured_a_binding, make_box("needs_box")), make_box(true)));
      CHECK(equal(runtime::get(captured_a_binding, make_box("has_boxed_usage")), make_box(true)));
      CHECK(equal(runtime::get(captured_a_binding, make_box("has_unboxed_usage")), make_box(false)));
    }
  }

  TEST_CASE("Boxed local")
  {
    runtime::context rt_ctx;

    SUBCASE("Boxed usage")
    {
      auto const res(rt_ctx.analyze_string("(let* [f (fn* [] 1) a (f)] a)"));
      CHECK_EQ(res.size(), 1);

      auto const map(res[0]->to_runtime_data());

      auto const f_binding(runtime::get_in(map, rt_ctx.eval_string(R"(["pairs" 1 1 "source_expr" "binding"])")));
      CHECK(equal(runtime::get(f_binding, make_box("needs_box")), make_box(true)));
      CHECK(equal(runtime::get(f_binding, make_box("has_boxed_usage")), make_box(true)));
      CHECK(equal(runtime::get(f_binding, make_box("has_unboxed_usage")), make_box(false)));

      auto const a_binding(runtime::get_in(map, rt_ctx.eval_string(R"(["pairs" 1 0])")));
      CHECK(equal(runtime::get(a_binding, make_box("needs_box")), make_box(true)));
      CHECK(equal(runtime::get(a_binding, make_box("has_boxed_usage")), make_box(true)));
      CHECK(equal(runtime::get(a_binding, make_box("has_unboxed_usage")), make_box(false)));
    }

    SUBCASE("Re-binding")
    {
      auto const res(rt_ctx.analyze_string("(let* [f (fn* [] 1) a f] a)"));
      CHECK_EQ(res.size(), 1);

      auto const map(res[0]->to_runtime_data());
      auto const a_binding(runtime::get_in(map, rt_ctx.eval_string(R"(["pairs" 1 0])")));

      CHECK(equal(runtime::get(a_binding, make_box("needs_box")), make_box(true)));
      CHECK(equal(runtime::get(a_binding, make_box("has_boxed_usage")), make_box(true)));
      CHECK(equal(runtime::get(a_binding, make_box("has_unboxed_usage")), make_box(false)));
    }
  }
}
