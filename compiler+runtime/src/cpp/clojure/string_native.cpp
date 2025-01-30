#include <fmt/format.h>

#include <jank/native_persistent_string/fmt.hpp>
#include <clojure/core_native.hpp>
#include <jank/runtime/convert.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/behavior/callable.hpp>
#include <jank/runtime/visit.hpp>

namespace clojure::string_native
{
  using namespace jank;
  using namespace jank::runtime;

  static object_ptr reverse(object_ptr const s)
  {
    auto const s_str(runtime::to_string(s));
    return make_box<obj::persistent_string>(native_persistent_string{ s_str.rbegin(), s_str.rend() });
  }

  static object_ptr starts_with(object_ptr const s, object_ptr const substr)
  {
    auto const s_str(runtime::to_string(s));
    auto const substr_str(runtime::to_string(substr));
    return make_box(s_str.starts_with(substr_str));
  }

  static object_ptr ends_with(object_ptr const s, object_ptr const substr)
  {
    auto const s_str(runtime::to_string(s));
    auto const substr_str(runtime::to_string(substr));
    return make_box(s_str.ends_with(substr_str));
  }
}

jank_object_ptr jank_load_clojure_string_native()
{
  using namespace jank;
  using namespace jank::runtime;
  using namespace clojure;

  auto const ns(__rt_ctx->intern_ns("clojure.string-native"));

  auto const intern_fn([=](native_persistent_string const &name, auto const fn) {
    ns->intern_var(name)->bind_root(
      make_box<obj::native_function_wrapper>(convert_function(fn))
        ->with_meta(obj::persistent_hash_map::create_unique(std::make_pair(
          __rt_ctx->intern_keyword("name").expect_ok(),
          make_box(obj::symbol{ __rt_ctx->current_ns()->to_string(), name }.to_string())))));
  });

  intern_fn("reverse", &string_native::reverse);
  intern_fn("starts-with?", &string_native::starts_with);
  intern_fn("ends-with?", &string_native::ends_with);

  return erase(obj::nil::nil_const());
}
