#include <clojure/string_native.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/obj/jit_function.hpp>
#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/obj/persistent_vector.hpp>
#include <jank/runtime/obj/re_pattern.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/util/fmt.hpp>
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

  static jtl::immutable_string replace_first(jtl::immutable_string const &s,
                                             jtl::immutable_string const &match,
                                             jtl::immutable_string const &replacement)
  {
    auto const i(s.find(match));

    if(i == jtl::immutable_string::npos)
    {
      return s;
    }

    auto const s_size(s.size());
    auto const match_size(match.size());
    auto const replacement_size(replacement.size());

    jtl::string_builder buff{ s_size - match_size + replacement_size };
    buff(s.substr(0, i));
    buff(replacement);

    auto const rest_i(i + match_size);

    if(rest_i < s_size)
    {
      buff(s.substr(rest_i));
    }

    return buff.release();
  }

  static jtl::immutable_string replace_first(jtl::immutable_string const &s,
                                             std::regex const &match,
                                             jtl::immutable_string const &replacement)
  {
    auto const out_str(std::regex_replace(s.c_str(),
                                          match,
                                          replacement.c_str(),
                                          std::regex_constants::format_first_only));

    return jtl::immutable_string{ out_str.c_str() };
  }

  static jtl::immutable_string replace_first(jtl::immutable_string const &s,
                                             std::regex const &match,
                                             object_ref const replacement)
  {
    std::smatch match_results{};
    std::string const search_str{ s.c_str() };
    std::regex_search(search_str, match_results, match);

    if(match_results.empty())
    {
      return s;
    }

    auto const i(match_results.position(0));

    jtl::string_builder buff;
    buff(s.substr(0, i));

    if(replacement->type == object_type::native_function_wrapper)
    {
      auto const replacement_fn(expect_object<obj::native_function_wrapper>(replacement));
      auto const replacement_value(replacement_fn->call(smatch_to_vector(match_results)));
      buff(try_object<obj::persistent_string>(replacement_value)->data);
    }
    else if(replacement->type == object_type::jit_function)
    {
      auto const replacement_fn(expect_object<obj::jit_function>(replacement));
      auto const replacement_value(replacement_fn->call(smatch_to_vector(match_results)));
      buff(try_object<obj::persistent_string>(replacement_value)->data);
    }
    else
    {
      throw std::runtime_error{ util::format("Invalid replacement arg: {}",
                                             runtime::to_code_string(replacement)) };
    }

    auto const rest_i(i + match_results[0].str().size());

    if(rest_i < s.size())
    {
      buff(s.substr(rest_i));
    }

    return buff.release();
  }

  static jtl::immutable_string replace_first(jtl::immutable_string const &s,
                                             object_ref const match,
                                             object_ref const replacement)
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"
    switch(match->type)
    {
      case object_type::character:
        return replace_first(s,
                             try_object<obj::character>(match)->data,
                             try_object<obj::character>(replacement)->data);
      case object_type::persistent_string:
        return replace_first(s,
                             try_object<obj::persistent_string>(match)->data,
                             try_object<obj::persistent_string>(replacement)->data);
      case object_type::re_pattern:
        if(replacement->type == object_type::persistent_string)
        {
          return replace_first(s,
                               try_object<obj::re_pattern>(match)->regex,
                               try_object<obj::persistent_string>(replacement)->data);
        }
        else
        {
          return replace_first(s, try_object<obj::re_pattern>(match)->regex, replacement);
        }
      default:
        throw std::runtime_error{ util::format("Invalid match arg: {}",
                                               runtime::to_code_string(match)) };
    }
#pragma clang diagnostic pop
  }

  object_ref replace_first(object_ref const s, object_ref const match, object_ref const replacement)
  {
    auto const is_string(s->type == object_type::persistent_string);
    auto const s_str(is_string ? try_object<obj::persistent_string>(s)->data
                               : runtime::to_string(s));

    auto const output_str(replace_first(s_str, match, replacement));

    return is_string && output_str == s_str ? s : make_box(output_str);
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
