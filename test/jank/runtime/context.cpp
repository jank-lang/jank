#include <doctest/doctest.h>

#include <jank/runtime/context.hpp>

namespace jank::runtime
{
  TEST_CASE("Initialization")
  {
    context ctx;
    ctx.initialize();
    CHECK(ctx.namespaces.find(type::symbol::create("clojure.core")) != ctx.namespaces.end());
    CHECK(ctx.namespaces.find(type::symbol::create("missing")) == ctx.namespaces.end());
    CHECK(ctx.current_ns->root->as_ns()->name->equal(type::symbol("clojure.core")));
  }

  TEST_CASE("Namespace changing")
  {
    context ctx;
    ctx.initialize();
    CHECK(ctx.current_ns->root->as_ns()->name->equal(type::symbol("clojure.core")));
    CHECK(ctx.namespaces.find(type::symbol::create("test")) == ctx.namespaces.end());
    ctx.in_ns->root->as_callable()->call(type::symbol::create("test"));
    CHECK(ctx.namespaces.find(type::symbol::create("test")) != ctx.namespaces.end());
    CHECK(ctx.current_ns->root->as_ns()->name->equal(type::symbol("test")));
  }
}
