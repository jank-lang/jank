#include <jank/runtime/context.hpp>
#include <jank/runtime/behavior/callable.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::runtime
{
  TEST_SUITE("runtime::context")
  {
    TEST_CASE("Initialization")
    {
      context ctx;
      auto const locked_namespaces(ctx.namespaces.rlock());
      CHECK(locked_namespaces->find(make_box<obj::symbol>("clojure.core"))
            != locked_namespaces->end());
      CHECK(locked_namespaces->find(make_box<obj::symbol>("missing")) == locked_namespaces->end());
      CHECK(expect_object<ns>(ctx.current_ns_var->get_root())
              ->name->equal(obj::symbol("", "clojure.core")));
    }

    TEST_CASE("Namespace changing")
    {
      context ctx;
      ctx.load_module("/clojure.core").expect_ok();

      {
        auto const locked_namespaces(ctx.namespaces.rlock());
        CHECK(expect_object<ns>(ctx.current_ns_var->get_root())
                ->name->equal(obj::symbol("", "clojure.core")));
        CHECK(locked_namespaces->find(make_box<obj::symbol>("test")) == locked_namespaces->end());
      }

      dynamic_call(ctx.in_ns_var->get_root(), make_box<obj::symbol>("test"));

      {
        auto const locked_namespaces(ctx.namespaces.rlock());
        CHECK(locked_namespaces->find(make_box<obj::symbol>("test")) != locked_namespaces->end());
        CHECK(expect_object<ns>(ctx.current_ns_var->deref())->name->equal(obj::symbol("", "test")));
      }
    }
  }
}
