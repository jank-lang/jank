#include <gc/gc.h>
#include <gc/gc_cpp.h>
#include <nanobench.h>

#include <old.hpp>
#include <new.hpp>

int main()
{
  GC_enable_incremental();

  std::cout << "object size [] " << sizeof(new_model::typed_object<new_model::behavior_type_none, new_model::storage_type_none>::data) << std::endl;
  std::cout << "object size [int] " << sizeof(new_model::typed_object<new_model::behavior_type_none, new_model::storage_type_integer>::data) << std::endl;
  std::cout << "object size [int, map] " << sizeof(new_model::typed_object<new_model::behavior_type_none, new_model::storage_type_integer | new_model::storage_type_map>::data) << std::endl;

  ankerl::nanobench::Config config;
  config.mMinEpochIterations = 1000000;
  config.mOut = &std::cout;
  config.mWarmup = 10000;

  ankerl::nanobench::Bench().config(config).run
  (
    "old map ctor",
    [&]
    {
      auto const ret(new (GC) old_model::map{});
      ankerl::nanobench::doNotOptimizeAway(ret);
    }
  );

  ankerl::nanobench::Bench().config(config).run
  (
    "new map ctor",
    [&]
    {
      auto const ret(new_model::make_object<new_model::behavior_type_map, new_model::storage_type_map | new_model::storage_type_metadatable>());
      ankerl::nanobench::doNotOptimizeAway(ret);
    }
  );
}
