#include <jank/runtime/context.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <doctest/doctest.h>
#pragma clang diagnostic pop

namespace jank::runtime
{
  TEST_CASE("Initialization")
  {
    context ctx;
    auto const locked_namespaces(ctx.namespaces.rlock());
    CHECK(locked_namespaces->find(obj::symbol::create("clojure.core")) != locked_namespaces->end());
    CHECK(locked_namespaces->find(obj::symbol::create("missing")) == locked_namespaces->end());
    CHECK(ctx.get_thread_state().current_ns->get_root()->as_ns()->name->equal(obj::symbol("", "clojure.core")));
  }

  TEST_CASE("Namespace changing")
  {
    context ctx;
    {
      auto const locked_namespaces(ctx.namespaces.rlock());
      CHECK(ctx.get_thread_state().current_ns->get_root()->as_ns()->name->equal(obj::symbol("", "clojure.core")));
      CHECK(locked_namespaces->find(obj::symbol::create("test")) == locked_namespaces->end());
    }
    ctx.get_thread_state().in_ns->get_root()->as_callable()->call(obj::symbol::create("test"));
    {
      auto const locked_namespaces(ctx.namespaces.rlock());
      CHECK(locked_namespaces->find(obj::symbol::create("test")) != locked_namespaces->end());
      CHECK(ctx.get_thread_state().current_ns->get_root()->as_ns()->name->equal(obj::symbol("", "test")));
    }
  }
}
