#include <clojure/core_native.hpp>
#include <clojure/string_native.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/obj/keyword.hpp>
#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/obj/persistent_hash_map.hpp>
#include <jank/runtime/convert/function.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/util/string.hpp>

namespace clojure::string_native
{
  using namespace jank;
  using namespace jank::runtime;

  static object_ref blank(object_ref const s)
  {
    if(runtime::is_nil(s))
    {
      return jank_true;
    }
    auto const s_str(runtime::to_string(s));
    return make_box(s_str.is_blank());
  }

  static object_ref reverse(object_ref const s)
  {
    auto const s_str(runtime::to_string(s));
    return make_box<obj::persistent_string>(jtl::immutable_string{ s_str.rbegin(), s_str.rend() });
  }

  static object_ref lower_case(object_ref const s)
  {
    auto const s_str(runtime::to_string(s));
    return make_box(util::to_lowercase(s_str));
  }

  static object_ref starts_with(object_ref const s, object_ref const substr)
  {
    auto const s_str(runtime::to_string(s));
    auto const substr_str(runtime::to_string(substr));
    return make_box(s_str.starts_with(substr_str));
  }

  static object_ref ends_with(object_ref const s, object_ref const substr)
  {
    auto const s_str(runtime::to_string(s));
    auto const substr_str(runtime::to_string(substr));
    return make_box(s_str.ends_with(substr_str));
  }

  static object_ref includes(object_ref const s, object_ref const substr)
  {
    auto const s_str(runtime::to_string(s));
    auto const substr_str(runtime::to_string(substr));
    return make_box(s_str.contains(substr_str));
  }

  static object_ref upper_case(object_ref const s)
  {
    auto const s_str(runtime::to_string(s));
    return make_box(util::to_uppercase(s_str));
  }
}

jank_object_ref jank_load_clojure_string_native()
{
  using namespace jank;
  using namespace jank::runtime;
  using namespace clojure;

  auto const ns(__rt_ctx->intern_ns("clojure.string-native"));

  auto const intern_fn([=](jtl::immutable_string const &name, auto const fn) {
    ns->intern_var(name)->bind_root(
      make_box<obj::native_function_wrapper>(convert_function(fn))
        ->with_meta(obj::persistent_hash_map::create_unique(std::make_pair(
          __rt_ctx->intern_keyword("name").expect_ok(),
          make_box(obj::symbol{ __rt_ctx->current_ns()->to_string(), name }.to_string())))));
  });

  intern_fn("blank?", &string_native::blank);
  intern_fn("ends-with?", &string_native::ends_with);
  intern_fn("includes?", &string_native::includes);
  intern_fn("lower-case", &string_native::lower_case);
  intern_fn("reverse", &string_native::reverse);
  intern_fn("starts-with?", &string_native::starts_with);
  intern_fn("upper-case", &string_native::upper_case);

  return jank_nil.erase();
}
