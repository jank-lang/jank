#include <jank/perf_native.hpp>
#include <jank/runtime/convert/function.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/munge.hpp>
#include <jank/runtime/perf.hpp>
#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/obj/persistent_hash_map.hpp>
#include <jank/runtime/obj/keyword.hpp>
#include <jank/runtime/rtti.hpp>

jank_object_ptr jank_load_jank_perf_native()
{
  using namespace jank;
  using namespace jank::runtime;

  auto const ns(__rt_ctx->intern_ns("jank.perf-native"));

  auto const intern_fn([=](jtl::immutable_string const &name, auto const fn) {
    ns->intern_var(name)->bind_root(
      make_box<obj::native_function_wrapper>(convert_function(fn))
        ->with_meta(obj::persistent_hash_map::create_unique(std::make_pair(
          __rt_ctx->intern_keyword("name").expect_ok(),
          make_box(obj::symbol{ __rt_ctx->current_ns()->to_string(), name }.to_string())))));
  });
  intern_fn("benchmark", &perf::benchmark);

  return erase(obj::nil::nil_const());
}
