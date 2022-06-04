#include <doctest/doctest.h>

#include <jank/runtime/context.hpp>

namespace jank::runtime
{
  TEST_CASE("Initialization")
  {
    context ctx;
    CHECK(ctx.namespaces.find(obj::symbol::create("clojure.core")) != ctx.namespaces.end());
    CHECK(ctx.namespaces.find(obj::symbol::create("missing")) == ctx.namespaces.end());
    CHECK(ctx.current_ns->root->as_ns()->name->equal(obj::symbol("clojure.core")));
  }

  TEST_CASE("Namespace changing")
  {
    context ctx;
    CHECK(ctx.current_ns->root->as_ns()->name->equal(obj::symbol("clojure.core")));
    CHECK(ctx.namespaces.find(obj::symbol::create("test")) == ctx.namespaces.end());
    ctx.in_ns->root->as_callable()->call(obj::symbol::create("test"));
    CHECK(ctx.namespaces.find(obj::symbol::create("test")) != ctx.namespaces.end());
    CHECK(ctx.current_ns->root->as_ns()->name->equal(obj::symbol("test")));
  }
}
