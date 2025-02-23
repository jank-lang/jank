#include <jank/runtime/behavior/callable.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/obj/persistent_list.hpp>
#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/obj/symbol.hpp>

/* This must go last; doctest and glog both define CHECK and family. */
#include <doctest/doctest.h>

namespace jank::runtime
{
  TEST_SUITE("callable")
  {
    TEST_CASE("apply_to")
    {
      CHECK(equal(apply_to(__rt_ctx->find_var(make_box<obj::symbol>("clojure.core/str")).unwrap(),
                           make_box<obj::persistent_list>()),
                  make_box<obj::persistent_string>("")));
      CHECK(equal(apply_to(__rt_ctx->find_var(make_box<obj::symbol>("clojure.core/str")).unwrap(),
                           make_box<obj::persistent_list>(std::in_place, make_box('f'))),
                  make_box<obj::persistent_string>("f")));
      CHECK(
        equal(apply_to(__rt_ctx->find_var(make_box<obj::symbol>("clojure.core/str")).unwrap(),
                       make_box<obj::persistent_list>(std::in_place, make_box('f'), make_box('g'))),
              make_box<obj::persistent_string>("fg")));
      CHECK(equal(apply_to(__rt_ctx->find_var(make_box<obj::symbol>("clojure.core/str")).unwrap(),
                           make_box<obj::persistent_list>(std::in_place,
                                                          make_box('f'),
                                                          make_box('g'),
                                                          make_box('h'))),
                  make_box<obj::persistent_string>("fgh")));
      CHECK(equal(apply_to(__rt_ctx->find_var(make_box<obj::symbol>("clojure.core/str")).unwrap(),
                           make_box<obj::persistent_list>(std::in_place,
                                                          make_box('f'),
                                                          make_box('g'),
                                                          make_box('h'),
                                                          make_box('i'))),
                  make_box<obj::persistent_string>("fghi")));
      CHECK(equal(apply_to(__rt_ctx->find_var(make_box<obj::symbol>("clojure.core/str")).unwrap(),
                           make_box<obj::persistent_list>(std::in_place,
                                                          make_box('f'),
                                                          make_box('g'),
                                                          make_box('h'),
                                                          make_box('i'),
                                                          make_box('j'))),
                  make_box<obj::persistent_string>("fghij")));
      CHECK(equal(apply_to(__rt_ctx->find_var(make_box<obj::symbol>("clojure.core/str")).unwrap(),
                           make_box<obj::persistent_list>(std::in_place,
                                                          make_box('f'),
                                                          make_box('g'),
                                                          make_box('h'),
                                                          make_box('i'),
                                                          make_box('j'),
                                                          make_box('k'))),
                  make_box<obj::persistent_string>("fghijk")));
      CHECK(equal(apply_to(__rt_ctx->find_var(make_box<obj::symbol>("clojure.core/str")).unwrap(),
                           make_box<obj::persistent_list>(std::in_place,
                                                          make_box('f'),
                                                          make_box('g'),
                                                          make_box('h'),
                                                          make_box('i'),
                                                          make_box('j'),
                                                          make_box('k'),
                                                          make_box('l'))),
                  make_box<obj::persistent_string>("fghijkl")));
      CHECK(equal(apply_to(__rt_ctx->find_var(make_box<obj::symbol>("clojure.core/str")).unwrap(),
                           make_box<obj::persistent_list>(std::in_place,
                                                          make_box('f'),
                                                          make_box('g'),
                                                          make_box('h'),
                                                          make_box('i'),
                                                          make_box('j'),
                                                          make_box('k'),
                                                          make_box('l'),
                                                          make_box('m'))),
                  make_box<obj::persistent_string>("fghijklm")));
      CHECK(equal(apply_to(__rt_ctx->find_var(make_box<obj::symbol>("clojure.core/str")).unwrap(),
                           make_box<obj::persistent_list>(std::in_place,
                                                          make_box('f'),
                                                          make_box('g'),
                                                          make_box('h'),
                                                          make_box('i'),
                                                          make_box('j'),
                                                          make_box('k'),
                                                          make_box('l'),
                                                          make_box('m'),
                                                          make_box('n'))),
                  make_box<obj::persistent_string>("fghijklmn")));
      CHECK(equal(apply_to(__rt_ctx->find_var(make_box<obj::symbol>("clojure.core/str")).unwrap(),
                           make_box<obj::persistent_list>(std::in_place,
                                                          make_box('f'),
                                                          make_box('g'),
                                                          make_box('h'),
                                                          make_box('i'),
                                                          make_box('j'),
                                                          make_box('k'),
                                                          make_box('l'),
                                                          make_box('m'),
                                                          make_box('n'),
                                                          make_box('o'))),
                  make_box<obj::persistent_string>("fghijklmno")));
      CHECK(equal(apply_to(__rt_ctx->find_var(make_box<obj::symbol>("clojure.core/str")).unwrap(),
                           make_box<obj::persistent_list>(std::in_place,
                                                          make_box('f'),
                                                          make_box('g'),
                                                          make_box('h'),
                                                          make_box('i'),
                                                          make_box('j'),
                                                          make_box('k'),
                                                          make_box('l'),
                                                          make_box('m'),
                                                          make_box('n'),
                                                          make_box('o'),
                                                          make_box('p'))),
                  make_box<obj::persistent_string>("fghijklmnop")));
      CHECK(equal(apply_to(__rt_ctx->find_var(make_box<obj::symbol>("clojure.core/str")).unwrap(),
                           make_box<obj::persistent_list>(std::in_place,
                                                          make_box('f'),
                                                          make_box('g'),
                                                          make_box('h'),
                                                          make_box('i'),
                                                          make_box('j'),
                                                          make_box('k'),
                                                          make_box('l'),
                                                          make_box('m'),
                                                          make_box('n'),
                                                          make_box('o'),
                                                          make_box('p'),
                                                          make_box('q'))),
                  make_box<obj::persistent_string>("fghijklmnopq")));
    }
  }
}
