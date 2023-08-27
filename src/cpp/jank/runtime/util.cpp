#include <unordered_map>

#include <jank/runtime/util.hpp>
#include <jank/runtime/obj/string.hpp>

namespace jank::runtime
{
  namespace detail
  {
    bool truthy(object_ptr const o)
    {
      if(!o)
      { return false; }

      return visit_object
      (
        [](auto const typed_o)
        {
          using T = typename decltype(typed_o)::value_type;

          if constexpr(std::same_as<T, obj::nil>)
          { return false; }
          else if constexpr(std::same_as<T, obj::boolean>)
          { return typed_o->data; }
          else
          { return true; }
        },
        o
      );
    }
    bool truthy(obj::nil_ptr)
    { return false; }
    bool truthy(obj::boolean_ptr const o)
    { return o && o->data; }
    bool truthy(native_bool const o)
    { return o; }
  }

  static native_unordered_map<char, native_string_view> const munge_chars
  {
    { '-', "_" },
    { ':', "_COLON_" },
    { '+', "_PLUS_" },
    { '>', "_GT_" },
    { '<', "_LT_" },
    { '=', "_EQ_" },
    { '~', "_TILDE_" },
    { '!', "_BANG_" },
    { '@', "_CIRCA_" },
    { '#', "_SHARP_" },
    { '\'', "_SINGLEQUOTE_" },
    { '"', "_DOUBLEQUOTE_" },
    { '%', "_PERCENT_" },
    { '^', "_CARET_" },
    { '&', "_AMPERSAND_" },
    { '*', "_STAR_" },
    { '|', "_BAR_" },
    { '{', "_LBRACE_" },
    { '}', "_RBRACE_" },
    { '[', "_LBRACK_" },
    { ']', "_RBRACK_" },
    { '/', "_SLASH_" },
    { '\\', "_BSLASH_" },
    { '?', "_QMARK_" }
  };

  native_string munge(native_string const &o)
  {
    native_string munged;
    for(auto const &c : o)
    {
      auto const &replacement(munge_chars.find(c));
      if(replacement != munge_chars.end())
      { munged.append(replacement->second); }
      else
      { munged.append(1, c); }
    }

    return munged;
  }

  /* TODO: Support symbols and other data; Clojure takes in anything and passes it through str. */
  object_ptr munge(object_ptr o)
  {
    return visit_object
    (
      [](auto const typed_o) -> object_ptr
      {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(std::same_as<T, obj::string>)
        { return jank::make_box<obj::string>(munge(typed_o->data)); }
        else
        { throw "munging only supported for strings right now"; }
      },
      o
    );
  }
}
