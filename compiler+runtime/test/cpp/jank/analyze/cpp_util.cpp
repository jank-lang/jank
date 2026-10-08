#include <doctest/doctest.h>

#include <jank/analyze/cpp_util.hpp>
#include <jank/runtime/context.hpp>

TEST_SUITE("analyze/cpp_util")
{
  using namespace jank::analyze;

  TEST_CASE("resolve_candidates orders candidates by relevance")
  {
    jank::runtime::__rt_ctx
      ->eval_cpp_string(R"cpp(
      namespace jank::test::resolve_candidates
      {
        int foo(int);
        int foo(double*);
        int foo(int, int);
        int foo() = delete;
      }
    )cpp")
      .expect_ok();

    auto const scope{ cppinterop::get_scope_from_complete_name("jank::test::resolve_candidates") };
    REQUIRE(scope);

    auto const fns{ cppinterop::get_functions_using_name(scope, "foo") };
    REQUIRE(fns.size() == 4);

    std::vector<cppinterop::clang_type> const arg_types{ { cpp_util::int_type() } };
    std::vector<cppinterop::clang_decl> const arg_scopes{ nullptr };
    auto const candidates{ cpp_util::resolve_candidates(fns, arg_types, arg_scopes, false) };

    REQUIRE(candidates.size() == 4);
    jank::usize one_arg_index{};
    jank::usize conversion_index{};
    jank::usize arity_index{};
    jank::usize deleted_index{};
    for(jank::usize i{}; i < fns.size(); ++i)
    {
      auto const num_args{ cppinterop::get_function_num_args(fns[i]) };
      if(num_args == 0)
      {
        deleted_index = i;
      }
      else if(num_args == 2)
      {
        arity_index = i;
      }
      else if(cppinterop::get_function_arg_type(fns[i], 0) == cpp_util::int_type())
      {
        one_arg_index = i;
      }
      else
      {
        conversion_index = i;
      }
    }
    auto const one_arg_signature{ cppinterop::get_function_signature(fns[one_arg_index]) };
    auto const conversion_signature{ cppinterop::get_function_signature(fns[conversion_index]) };
    auto const arity_signature{ cppinterop::get_function_signature(fns[arity_index]) };
    auto const deleted_signature{ cppinterop::get_function_signature(fns[deleted_index]) };
    CHECK(candidates[0].signature == one_arg_signature);
    CHECK(candidates[1].signature == deleted_signature);
    CHECK(candidates[2].signature == conversion_signature);
    CHECK(candidates[3].signature == arity_signature);
  }

  TEST_CASE("resolve_candidates ranks access/const violations above conversion failures")
  {
    auto const locked_interpreter{ jank::runtime::__rt_ctx->jit_prc.interpreter.lock() };
    jank::runtime::__rt_ctx
      ->eval_cpp_string(R"cpp(
      namespace jank::test::resolve_candidates_members
      {
        struct widget
        {
          public:
            void run(int);
          private:
            void run(double);
        };
      }
    )cpp")
      .expect_ok();

    auto const scope{ cppinterop::get_scope_from_complete_name(
      "jank::test::resolve_candidates_members::widget") };
    REQUIRE(scope);

    auto const fns{ cppinterop::get_functions_using_name(scope, "run") };
    REQUIRE(fns.size() == 2);

    /* The implicit object parameter is const, so the public `run(int)` overload
     * (which is non-const) is a const_mismatch, while the private `run(double)`
     * overload is an access_violation regardless of constness. access_violation
     * must rank above const_mismatch. */
    auto const widget_type{ cppinterop::get_type_from_scope(scope) };
    REQUIRE(widget_type);
    auto const const_widget_type{ cppinterop::get_type_with_const(widget_type) };
    REQUIRE(const_widget_type);

    std::vector<cppinterop::clang_type> const arg_types{ { const_widget_type },
                                                         { cpp_util::int_type() } };
    std::vector<cppinterop::clang_decl> const arg_scopes{ nullptr, nullptr };
    auto const candidates{ cpp_util::resolve_candidates(fns, arg_types, arg_scopes, true) };

    REQUIRE(candidates.size() == 2);
    jank::usize private_index{};
    jank::usize public_index{};
    for(jank::usize i{}; i < fns.size(); ++i)
    {
      if(cppinterop::is_private_method(fns[i]))
      {
        private_index = i;
      }
      else
      {
        public_index = i;
      }
    }
    auto const private_signature{ cppinterop::get_function_signature(fns[private_index]) };
    auto const public_signature{ cppinterop::get_function_signature(fns[public_index]) };
    CHECK(candidates[0].signature == private_signature);
    CHECK(candidates[1].signature == public_signature);
  }
}
