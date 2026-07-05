#include <jtl/utf8.hpp>

#include <clojure/string_native.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/core/call.hpp>
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

  bool blank(object_ref const s)
  {
    if(runtime::is_nil(s))
    {
      return true;
    }
    auto const s_str(runtime::to_string(s));
    return s_str.is_blank();
  }

  jtl::immutable_string reverse(jtl::immutable_string const &s)
  {
    jtl::string_builder buff{ s.size() };
    for(auto const &c : jtl::utf8_reverse_range(s))
    {
      buff(c);
    }
    return buff.release();
  }

  jtl::immutable_string lower_case(jtl::immutable_string const &s)
  {
    return util::to_lowercase(s);
  }

  bool starts_with(jtl::immutable_string const &s, jtl::immutable_string const &substr)
  {
    return s.starts_with(substr);
  }

  bool ends_with(jtl::immutable_string const &s, jtl::immutable_string const &substr)
  {
    return s.ends_with(substr);
  }

  bool includes(jtl::immutable_string const &s, jtl::immutable_string const &substr)
  {
    return s.contains(substr);
  }

  jtl::immutable_string upper_case(jtl::immutable_string const &s)
  {
    return util::to_uppercase(s);
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

    auto const group(smatch_to_vector(match_results));
    auto const replacement_value(replacement.call(group));
    buff(try_object<obj::persistent_string>(replacement_value)->data);

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
    switch(match.get_type())
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
        if(replacement.get_type() == object_type::persistent_string)
        {
          return replace_first(s,
                               try_object<obj::re_pattern>(match)->regex,
                               try_object<obj::persistent_string>(replacement)->data);
        }

        return replace_first(s, try_object<obj::re_pattern>(match)->regex, replacement);
      default:
        throw std::runtime_error{ util::format("Invalid match arg: {}",
                                               runtime::to_code_string(match)) };
    }
#pragma clang diagnostic pop
  }

  object_ref replace_first(object_ref const s, object_ref const match, object_ref const replacement)
  {
    auto const is_string(s.get_type() == object_type::persistent_string);
    auto const &s_str(is_string ? try_object<obj::persistent_string>(s)->data
                                : runtime::to_string(s));

    auto const output_str(replace_first(s_str, match, replacement));

    return is_string && output_str == s_str ? s : make_box(output_str);
  }

  i64 index_of(jtl::immutable_string const &s,
               jtl::immutable_string const &value,
               i64 const from_index)
  {
    return static_cast<i64>(s.find(value, from_index));
  }

  i64 last_index_of(jtl::immutable_string const &s,
                    jtl::immutable_string const &value,
                    i64 const from_index)
  {
    return static_cast<i64>(s.rfind(value, from_index));
  }

  static jtl::immutable_string::size_type triml_index(jtl::immutable_string const &s)
  {
    auto const s_size(s.size());
    jtl::immutable_string::size_type i{ 0 };

    for(; i < s_size; ++i)
    {
      if(!std::isspace(s[i]))
      {
        break;
      }
    }

    return i;
  }

  jtl::immutable_string triml(jtl::immutable_string const &s)
  {
    auto const l(triml_index(s));

    if(l == 0)
    {
      return s;
    }

    if(l == s.size())
    {
      return {};
    }

    return s.substr(l);
  }

  static jtl::immutable_string::size_type trimr_index(jtl::immutable_string const &s)
  {
    auto const s_size(s.size());
    jtl::immutable_string::size_type i{ s_size };

    for(; i > 0; --i)
    {
      if(!std::isspace(s[i - 1]))
      {
        break;
      }
    }

    return i;
  }

  jtl::immutable_string trimr(jtl::immutable_string const &s)
  {
    auto const r(trimr_index(s));

    if(r == s.size())
    {
      return s;
    }

    if(r == 0)
    {
      return {};
    }

    return s.substr(0, r);
  }

  jtl::immutable_string trim(jtl::immutable_string const &s)
  {
    auto const r(trimr_index(s));

    if(r == 0)
    {
      return {};
    }

    auto const l(triml_index(s));

    if(l == 0 && r == s.size())
    {
      return s;
    }

    return s.substr(l, r - l);
  }

  static jtl::immutable_string::size_type trim_newline_index(jtl::immutable_string const &s)
  {
    auto const s_size(s.size());
    jtl::immutable_string::size_type i{ s_size };

    for(; i > 0; --i)
    {
      auto const c(s[i - 1]);
      if(c != '\n' && c != '\r')
      {
        break;
      }
    }

    return i;
  }

  jtl::immutable_string trim_newline(jtl::immutable_string const &s)
  {
    auto const r(trim_newline_index(s));

    if(r == s.size())
    {
      return s;
    }

    return s.substr(0, r);
  }

  obj::persistent_vector_ref split(jtl::immutable_string const &s, obj::re_pattern_ref const re)
  {
    auto const regex(re->regex);

    std::string const search_str{ s.c_str() };

    detail::native_transient_vector vec;
    std::sregex_token_iterator iter(search_str.begin(), search_str.end(), regex, -1);
    std::sregex_token_iterator const end;

    for(; iter != end; ++iter)
    {
      vec.push_back(make_box<obj::persistent_string>(iter->str()));
    }

    return make_box<obj::persistent_vector>(vec.persistent());
  }

  obj::persistent_vector_ref
  split(jtl::immutable_string const &s, obj::re_pattern_ref const re, i64 const limit)
  {
    if(limit < 1)
    {
      return split(s, re);
    }

    auto const regex(re->regex);

    std::string const search_str{ s.c_str() };

    detail::native_transient_vector vec;

    std::sregex_token_iterator iter(search_str.begin(), search_str.end(), regex, -1);
    std::sregex_token_iterator const end;

    int i{ 1 };
    for(; i < limit && iter != end; ++i, ++iter)
    {
      vec.push_back(make_box<obj::persistent_string>(iter->str().c_str()));
    }

    if(i == limit)
    {
      vec.push_back(make_box(s.substr(iter->first - search_str.begin())));
    }

    return make_box<obj::persistent_vector>(vec.persistent());
  }
}
