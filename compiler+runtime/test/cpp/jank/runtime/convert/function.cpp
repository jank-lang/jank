#include <functional>

#include <jank/runtime/convert/function.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/equal.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/obj/opaque_box.hpp>
#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/obj/symbol.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::runtime
{
  /* A non-trivial native type which isn't trait-convertible. Used to verify
   * the opaque box fallback when passing native args through jank fns. */
  struct test_widget
  {
    int value{};
  };

  TEST_SUITE("convert<std::function>")
  {
    TEST_CASE("from_object: void return, no args")
    {
      auto const inc_var{ __rt_ctx->find_var(make_box<obj::symbol>("clojure.core/inc")) };
      CHECK(is_callable(inc_var));

      auto const fn{ convert<std::function<void()>>::from_object(inc_var) };
      /* Conversion itself should succeed even though invoking without args
       * would fail for clojure.core/inc. We just want to verify we get back
       * a valid std::function here. */
      CHECK(fn.operator bool());
    }

    TEST_CASE("from_object: non-void return, primitive arg")
    {
      auto const inc_var{ __rt_ctx->find_var(make_box<obj::symbol>("clojure.core/inc")) };
      auto const fn{ convert<std::function<object_ref(object_ref)>>::from_object(inc_var) };
      auto const result{ fn(make_box(41)) };
      CHECK(equal(result, make_box(42)));
    }

    TEST_CASE("from_object: native int arg converted via trait")
    {
      auto const inc_var{ __rt_ctx->find_var(make_box<obj::symbol>("clojure.core/inc")) };
      auto const fn{ convert<std::function<i64(i64)>>::from_object(inc_var) };
      CHECK(fn(41) == 42);
    }

    TEST_CASE("from_object: multiple primitive args")
    {
      auto const plus_var{ __rt_ctx->find_var(make_box<obj::symbol>("clojure.core/+")) };
      auto const fn{ convert<std::function<i64(i64, i64)>>::from_object(plus_var) };
      CHECK(fn(20, 22) == 42);
    }

    TEST_CASE("from_object: trait-convertible string arg")
    {
      auto const str_var{ __rt_ctx->find_var(make_box<obj::symbol>("clojure.core/str")) };
      auto const fn{ convert<std::function<std::string(std::string)>>::from_object(str_var) };
      CHECK(fn("meow") == "meow");
    }

    TEST_CASE("from_object: non-convertible arg falls back to opaque box")
    {
      /* clojure.core/identity just returns whatever it's given. The interesting
       * bit is that the non-trait-convertible test_widget is boxed into an
       * opaque_box on the way in. */
      auto const identity_var{ __rt_ctx->find_var(make_box<obj::symbol>("clojure.core/identity")) };
      auto const fn{ convert<std::function<object_ref(test_widget const &)>>::from_object(
        identity_var) };

      test_widget w{ 42 };
      auto const result{ fn(w) };
      auto const box{ dyn_cast<obj::opaque_box>(result) };
      CHECK(box.is_some());
      CHECK(box->canonical_type == "jank::runtime::test_widget*");
      CHECK(static_cast<test_widget *>(static_cast<void *>(box->data))->value == 42);
    }

    TEST_CASE("from_object: opaque box type name drops const")
    {
      auto const identity_var{ __rt_ctx->find_var(make_box<obj::symbol>("clojure.core/identity")) };
      auto const fn{ convert<std::function<object_ref(test_widget const &)>>::from_object(
        identity_var) };

      test_widget w{};
      auto const result{ fn(w) };
      auto const box{ dyn_cast<obj::opaque_box>(result) };
      CHECK(box.is_some());
      /* The tag must match what codegen emits for cpp/unbox of the
       * corresponding pointer type, which strips top-level const. */
      CHECK(box->canonical_type == "jank::runtime::test_widget*");
    }

    TEST_CASE("from_object: non-callable throws")
    {
      CHECK_THROWS(convert<std::function<void()>>::from_object(make_box(42)));
      CHECK_THROWS(convert<std::function<void()>>::from_object(make_box("not a fn")));
    }

    TEST_CASE("into_object: wraps std::function in native_function_wrapper")
    {
      std::function<object_ref(object_ref)> const f{ [](object_ref const o) { return o; } };
      auto const obj{ convert<std::function<object_ref(object_ref)>>::into_object(f) };
      CHECK(obj.get_type() == object_type::native_function_wrapper);

      auto const wrapped{ dyn_cast<obj::native_function_wrapper>(obj) };
      CHECK(wrapped.is_some());
      CHECK(equal(wrapped->call(make_box(7)), make_box(7)));
    }

    TEST_CASE("into_object: native function wrapper roundtrip")
    {
      std::function<i64(i64)> const f{ [](i64 const x) { return x * 2; } };
      /* First, wrap into a native_function_wrapper. Note that native_function_wrapper
       * requires the std::function to take object_ref args, so we need to first
       * convert the signature. We do this by going through into_object on an
       * already-converted std::function. */
      std::function<object_ref(object_ref)> const wrapper{
        [f](object_ref const arg) -> object_ref {
          return convert<i64>::into_object(f(convert<i64>::from_object(arg)));
        }
      };
      auto const obj{ convert<std::function<object_ref(object_ref)>>::into_object(wrapper) };

      /* Now convert back to a std::function<i64(i64)> and verify it works. */
      auto const back{ convert<std::function<i64(i64)>>::from_object(obj) };
      CHECK(back(21) == 42);
    }
  }
}
