#include <doctest/doctest.h>

#include <CppInterOp/Compatibility.h>
#include <CppInterOp/CppInterOp.h>

#include <jank/analyze/cpp_util.hpp>
#include <jank/runtime/context.hpp>

TEST_SUITE("analyze/cpp_util")
{
  TEST_CASE("rank_candidates orders candidates by relevance")
  {
    auto parse_result{ jank::runtime::__rt_ctx->jit_prc.interpreter->Parse(R"cpp(
      namespace jank::test::rank_candidates
      {
        int foo(int);
        int foo(double*);
        int foo(int, int);
        int foo() = delete;
      }
    )cpp") };
    auto const parse_succeeded{ static_cast<bool>(parse_result) };
    REQUIRE(parse_succeeded);

    auto const scope{ Cpp::GetScopeFromCompleteName("jank::test::rank_candidates") };
    REQUIRE(scope);

    auto const fns{ Cpp::GetFunctionsUsingName(scope, "foo") };
    REQUIRE(fns.size() == 4);

    std::vector<Cpp::TemplateArgInfo> const arg_types{ { jank::analyze::cpp_util::int_type() } };
    std::vector<Cpp::TCppScope_t> const arg_scopes{ nullptr };
    auto const candidates{ jank::analyze::cpp_util::rank_candidates(fns, arg_types, arg_scopes) };

    REQUIRE(candidates.size() == 4);
    jank::usize one_arg_index{};
    jank::usize conversion_index{};
    jank::usize arity_index{};
    jank::usize deleted_index{};
    for(jank::usize i{}; i < fns.size(); ++i)
    {
      auto const num_args{ Cpp::GetFunctionNumArgs(fns[i]) };
      if(num_args == 0)
      {
        deleted_index = i;
      }
      else if(num_args == 2)
      {
        arity_index = i;
      }
      else if(Cpp::GetFunctionArgType(fns[i], 0) == jank::analyze::cpp_util::int_type())
      {
        one_arg_index = i;
      }
      else
      {
        conversion_index = i;
      }
    }
    auto const one_arg_signature{ Cpp::GetFunctionSignature(fns[one_arg_index]) };
    auto const conversion_signature{ Cpp::GetFunctionSignature(fns[conversion_index]) };
    auto const arity_signature{ Cpp::GetFunctionSignature(fns[arity_index]) };
    auto const deleted_signature{ Cpp::GetFunctionSignature(fns[deleted_index]) };
    CHECK(candidates[0].signature == one_arg_signature);
    CHECK(candidates[1].signature == deleted_signature);
    CHECK(candidates[2].signature == conversion_signature);
    CHECK(candidates[3].signature == arity_signature);
  }

  TEST_CASE("rank_candidates ranks access/const violations above conversion failures")
  {
    auto parse_result{ jank::runtime::__rt_ctx->jit_prc.interpreter->Parse(R"cpp(
      namespace jank::test::rank_candidates_members
      {
        struct widget
        {
          public:
            void run(int);
          private:
            void run(double);
        };
      }
    )cpp") };
    auto const parse_succeeded{ static_cast<bool>(parse_result) };
    REQUIRE(parse_succeeded);

    auto const scope{ Cpp::GetScopeFromCompleteName(
      "jank::test::rank_candidates_members::widget") };
    REQUIRE(scope);

    auto const fns{ Cpp::GetFunctionsUsingName(scope, "run") };
    REQUIRE(fns.size() == 2);

    /* The implicit object parameter is const, so the public `run(int)` overload
     * (which is non-const) is a const_mismatch, while the private `run(double)`
     * overload is an access_violation regardless of constness. access_violation
     * must rank above const_mismatch. */
    auto const widget_type{ Cpp::GetTypeFromScope(scope) };
    REQUIRE(widget_type);
    auto const const_widget_type{ Cpp::GetTypeWithConst(widget_type) };
    REQUIRE(const_widget_type);

    std::vector<Cpp::TemplateArgInfo> const arg_types{ { const_widget_type },
                                                       { jank::analyze::cpp_util::int_type() } };
    std::vector<Cpp::TCppScope_t> const arg_scopes{ nullptr, nullptr };
    auto const candidates{ jank::analyze::cpp_util::rank_candidates(fns, arg_types, arg_scopes) };

    REQUIRE(candidates.size() == 2);
    jank::usize private_index{};
    jank::usize public_index{};
    for(jank::usize i{}; i < fns.size(); ++i)
    {
      if(Cpp::IsPrivateMethod(fns[i]))
      {
        private_index = i;
      }
      else
      {
        public_index = i;
      }
    }
    auto const private_signature{ Cpp::GetFunctionSignature(fns[private_index]) };
    auto const public_signature{ Cpp::GetFunctionSignature(fns[public_index]) };
    CHECK(candidates[0].signature == private_signature);
    CHECK(candidates[1].signature == public_signature);
  }
}
