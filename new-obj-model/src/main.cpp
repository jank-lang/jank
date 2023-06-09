#include <gc/gc.h>
#include <gc/gc_cpp.h>
#include <nanobench.h>

#include <jank/obj-model/inheritance/object.hpp>
#include <jank/obj-model/bitfield/object.hpp>

int main()
{
  GC_enable();

  std::cout << "object size [] " << sizeof(jank::obj_model::bitfield::typed_object<jank::obj_model::bitfield::behavior_type_none, jank::obj_model::bitfield::storage_type_none>::data) << std::endl;
  std::cout << "object size [int] " << sizeof(jank::obj_model::bitfield::typed_object<jank::obj_model::bitfield::behavior_type_none, jank::obj_model::bitfield::storage_type_integer>::data) << std::endl;
  std::cout << "object size [int, map] " << sizeof(jank::obj_model::bitfield::typed_object<jank::obj_model::bitfield::behavior_type_none, jank::obj_model::bitfield::storage_type_integer | jank::obj_model::bitfield::storage_type_map>::data) << std::endl;

  ankerl::nanobench::Config config;
  config.mMinEpochIterations = 1000000;
  config.mOut = &std::cout;
  config.mWarmup = 10000;

  ankerl::nanobench::Bench().config(config).run
  (
    "[inheritance] empty map ctor",
    [&]
    {
      auto const ret(new (GC) jank::obj_model::inheritance::map{});
      ankerl::nanobench::doNotOptimizeAway(ret);
    }
  );

  ankerl::nanobench::Bench().config(config).run
  (
    "[inheritance] ctor {nil nil}",
    [&]
    {
      auto const ret(new (GC) jank::obj_model::inheritance::map{ jank::runtime::detail::in_place_unique{}, new (GC) jank::runtime::object_ptr[2]{ jank::runtime::JANK_NIL, jank::runtime::JANK_NIL }, 2 });;
      ankerl::nanobench::doNotOptimizeAway(ret);
    }
  );

  auto const nil(jank::obj_model::bitfield::make_object<jank::obj_model::bitfield::behavior_type_nil, jank::obj_model::bitfield::storage_type_none>());
  ankerl::nanobench::Bench().config(config).run
  (
    "[bitfield] empty map ctor",
    [&]
    {
      auto const ret(jank::obj_model::bitfield::make_object<jank::obj_model::bitfield::behavior_type_map, jank::obj_model::bitfield::storage_type_map | jank::obj_model::bitfield::storage_type_metadatable>());
      ankerl::nanobench::doNotOptimizeAway(ret);
    }
  );

  ankerl::nanobench::Bench().config(config).run
  (
    "new int",
    [&]
    {
      auto const ret(new (PointerFreeGC) int{});
      ankerl::nanobench::doNotOptimizeAway(ret);
    }
  );
}
