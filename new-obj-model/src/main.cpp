#include <concepts>

#include <gc/gc.h>
#include <gc/gc_cpp.h>
#include <nanobench.h>

#include <jank/obj-model/inheritance/object.hpp>

#include <jank/obj-model/tagged/object.hpp>
#include <jank/obj-model/tagged/unerase.hpp>
#include <jank/obj-model/tagged/map.hpp>
#include <jank/obj-model/tagged/keyword.hpp>
#include <jank/obj-model/tagged/associatively_readable.hpp>

void benchmark_inheritance(ankerl::nanobench::Config const &config)
{
  using namespace jank::obj_model::inheritance;

  auto type_erase = [](jank::runtime::object_ptr const o)
  { return o; };

  ankerl::nanobench::Bench().config(config).run
  (
    "[inheritance] empty map ctor",
    [&]
    {
      auto const ret(new (GC) map{});
      ankerl::nanobench::doNotOptimizeAway(ret);
    }
  );

  ankerl::nanobench::Bench().config(config).run
  (
    "[inheritance] ctor {:a :b}",
    [&]
    {
      auto const ret(new (GC) map{ jank::runtime::detail::in_place_unique{}, new (GC) jank::runtime::object_ptr[2]{ jank::runtime::JANK_NIL, jank::runtime::JANK_NIL }, 2 });;
      ankerl::nanobench::doNotOptimizeAway(ret);
    }
  );

  auto const kw_a(jank::make_box<jank::runtime::obj::keyword>(jank::runtime::obj::symbol{ "", "a" }, true));
  auto const kw_b(jank::make_box<jank::runtime::obj::keyword>(jank::runtime::obj::symbol{ "", "b" }, true));
  auto const m(type_erase(new (GC) map{ jank::runtime::detail::in_place_unique{}, new (GC) jank::runtime::object_ptr[2]{ kw_a, kw_b }, 2 }));

  size_t c{};
  ankerl::nanobench::Bench().config(config).run
  (
    "[inheritance] map count",
    [&]
    { c = m->as_countable()->count(); }
  );
  assert(c == 2);

  jank::runtime::object_ptr found{};
  ankerl::nanobench::Bench().config(config).run
  (
    "[inheritance] map get",
    [&]
    {
      found = m->as_associatively_readable()->get(kw_a);
      ankerl::nanobench::doNotOptimizeAway(found);
    }
  );
  assert(found);
}

void benchmark_tagged(ankerl::nanobench::Config const &config)
{
  using namespace jank::obj_model::tagged;

  ankerl::nanobench::Bench().config(config).run
  (
    "[tagged] empty map ctor",
    [&]
    {
      auto const ret(static_map::create());
      ankerl::nanobench::doNotOptimizeAway(ret);
    }
  );

  auto const nil(static_nil::create());
  ankerl::nanobench::Bench().config(config).run
  (
    "[tagged] ctor {:a :b}",
    [&]
    {
      auto const ret(static_map::create(nil, nil));
      ankerl::nanobench::doNotOptimizeAway(ret);
    }
  );

  auto const kw_a(static_keyword::create(jank::runtime::obj::symbol{ "", "a" }, true));
  auto const kw_b(static_keyword::create(jank::runtime::obj::symbol{ "", "b" }, true));
  auto const map(erase_type(static_map::create(kw_a, kw_b)));
  size_t c{};
  ankerl::nanobench::Bench().config(config).run
  (
    "[tagged] map count",
    [&]
    {
      unerase_type
      (
        map,
        [&](auto * const typed_map)
        {
          using T = std::decay_t<decltype(typed_map)>;
          if constexpr(std::is_same_v<T, static_map*>)
          { c = typed_map->count(); }
        }
      );
    }
  );
  assert(c == 2);

  object_ptr res{};
  ankerl::nanobench::Bench().config(config).run
  (
    "[tagged] map get",
    [&]
    {
      unerase_type
      (
        map,
        [&](auto * const typed_map)
        {
          using T = std::decay_t<decltype(typed_map)>;
          if constexpr(jank::obj_model::tagged::associatively_readable<T>)
          { res = typed_map->get(erase_type(kw_a)); }
        }
      );
    }
  );
  assert(res);

}

int main()
{
  GC_enable();

  ankerl::nanobench::Config config;
  config.mMinEpochIterations = 1000000;
  config.mOut = &std::cout;
  config.mWarmup = 10000;

  benchmark_inheritance(config);
  benchmark_tagged(config);

  ankerl::nanobench::Bench().config(config).run
  (
    "new int",
    [&]
    {
      auto const ret(jank::make_box(1044));
      ankerl::nanobench::doNotOptimizeAway(ret);
    }
  );
}
