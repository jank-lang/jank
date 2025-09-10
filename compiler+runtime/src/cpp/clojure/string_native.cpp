#include <clojure/string_native.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/util/string.hpp>

namespace clojure::string_native
{
  using namespace jank;
  using namespace jank::runtime;

  object_ref blank(object_ref const s)
  {
    if(runtime::is_nil(s))
    {
      return jank_true;
    }
    auto const s_str(runtime::to_string(s));
    return make_box(s_str.is_blank());
  }

  object_ref reverse(object_ref const s)
  {
    auto const s_str(runtime::to_string(s));
    return make_box<obj::persistent_string>(jtl::immutable_string{ s_str.rbegin(), s_str.rend() });
  }

  object_ref lower_case(object_ref const s)
  {
    auto const s_str(runtime::to_string(s));
    return make_box(util::to_lowercase(s_str));
  }

  object_ref starts_with(object_ref const s, object_ref const substr)
  {
    auto const s_str(runtime::to_string(s));
    auto const substr_str(runtime::to_string(substr));
    return make_box(s_str.starts_with(substr_str));
  }

  object_ref ends_with(object_ref const s, object_ref const substr)
  {
    auto const s_str(runtime::to_string(s));
    auto const substr_str(runtime::to_string(substr));
    return make_box(s_str.ends_with(substr_str));
  }

  object_ref includes(object_ref const s, object_ref const substr)
  {
    auto const s_str(runtime::to_string(s));
    auto const substr_str(runtime::to_string(substr));
    return make_box(s_str.contains(substr_str));
  }

  object_ref upper_case(object_ref const s)
  {
    auto const s_str(runtime::to_string(s));
    return make_box(util::to_uppercase(s_str));
  }

  i64 index_of(object_ref const s, object_ref const value, object_ref const from_index)
  {
    auto const s_str(runtime::to_string(s));
    auto const value_str(runtime::to_string(value));
    auto const pos(try_object<obj::integer>(from_index)->data);
    return static_cast<i64>(s_str.find(value_str, pos));
  }

  i64 last_index_of(object_ref const s, object_ref const value, object_ref const from_index)
  {
    auto const s_str(runtime::to_string(s));
    auto const value_str(runtime::to_string(value));
    auto const pos(try_object<obj::integer>(from_index)->data);
    return static_cast<i64>(s_str.rfind(value_str, pos));
  }
}
