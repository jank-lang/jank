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
      {
        return false;
      }

      return visit_object(
        [](auto const typed_o) {
          using T = typename decltype(typed_o)::value_type;

          if constexpr(std::same_as<T, obj::nil>)
          {
            return false;
          }
          else if constexpr(std::same_as<T, obj::boolean>)
          {
            return typed_o->data;
          }
          else
          {
            return true;
          }
        },
        o);
    }

    bool truthy(obj::nil_ptr)
    {
      return false;
    }

    bool truthy(obj::boolean_ptr const o)
    {
      return o && o->data;
    }

    bool truthy(native_bool const o)
    {
      return o;
    }
  }

  static native_unordered_map<char, native_persistent_string_view> const munge_chars{
    { '-',             "_"},
    { ':',       "_COLON_"},
    { '+',        "_PLUS_"},
    { '>',          "_GT_"},
    { '<',          "_LT_"},
    { '=',          "_EQ_"},
    { '~',       "_TILDE_"},
    { '!',        "_BANG_"},
    { '@',       "_CIRCA_"},
    { '#',       "_SHARP_"},
    {'\'', "_SINGLEQUOTE_"},
    { '"', "_DOUBLEQUOTE_"},
    { '%',     "_PERCENT_"},
    { '^',       "_CARET_"},
    { '&',   "_AMPERSAND_"},
    { '*',        "_STAR_"},
    { '|',         "_BAR_"},
    { '{',      "_LBRACE_"},
    { '}',      "_RBRACE_"},
    { '[',      "_LBRACK_"},
    { ']',      "_RBRACK_"},
    { '/',       "_SLASH_"},
    {'\\',      "_BSLASH_"},
    { '?',       "_QMARK_"}
  };

  /* https://en.cppreference.com/w/cpp/keyword */
  static native_set<native_persistent_string_view> const cpp_keywords{ "alignas",
                                                                       "alignof",
                                                                       "and",
                                                                       "and_eq",
                                                                       "asm",
                                                                       "atomic_cancel",
                                                                       "atomic_commit",
                                                                       "atomic_noexcept",
                                                                       "auto",
                                                                       "bitand",
                                                                       "bitor",
                                                                       "bool",
                                                                       "break",
                                                                       "case",
                                                                       "catch",
                                                                       "char",
                                                                       "char8_t",
                                                                       "char16_t",
                                                                       "char32_t",
                                                                       "class",
                                                                       "compl",
                                                                       "concept",
                                                                       "const",
                                                                       "consteval",
                                                                       "constexpr",
                                                                       "constinit",
                                                                       "const_cast",
                                                                       "continue",
                                                                       "co_await",
                                                                       "co_return",
                                                                       "co_yield",
                                                                       "decltype",
                                                                       "default",
                                                                       "delete",
                                                                       "do",
                                                                       "double",
                                                                       "dynamic_cast",
                                                                       "else",
                                                                       "enum",
                                                                       "explicit",
                                                                       "export",
                                                                       "extern",
                                                                       "false",
                                                                       "float",
                                                                       "for",
                                                                       "friend",
                                                                       "goto",
                                                                       "if",
                                                                       "inline",
                                                                       "int",
                                                                       "long",
                                                                       "mutable",
                                                                       "namespace",
                                                                       "new",
                                                                       "noexcept",
                                                                       "not",
                                                                       "not_eq",
                                                                       "nullptr",
                                                                       "operator",
                                                                       "or",
                                                                       "or_eq",
                                                                       "private",
                                                                       "protected",
                                                                       "public",
                                                                       "reflexpr",
                                                                       "register",
                                                                       "reinterpret_cast",
                                                                       "requires",
                                                                       "return",
                                                                       "short",
                                                                       "signed",
                                                                       "sizeof",
                                                                       "static",
                                                                       "static_assert",
                                                                       "static_cast",
                                                                       "struct",
                                                                       "switch",
                                                                       "synchronized",
                                                                       "template",
                                                                       "this",
                                                                       "thread_local",
                                                                       "throw",
                                                                       "true",
                                                                       "try",
                                                                       "typedef",
                                                                       "typeid",
                                                                       "typename",
                                                                       "union",
                                                                       "unsigned",
                                                                       "using",
                                                                       "virtual",
                                                                       "void",
                                                                       "volatile",
                                                                       "wchar_t",
                                                                       "while",
                                                                       "xor",
                                                                       "xor_eq" };

  native_persistent_string munge(native_persistent_string const &o)
  {
    native_transient_string munged;
    for(auto const &c : o)
    {
      auto const &replacement(munge_chars.find(c));
      if(replacement != munge_chars.end())
      {
        munged.append(replacement->second);
      }
      else
      {
        munged.append(1, c);
      }
    }

    if(cpp_keywords.contains(munged))
    {
      munged += "__";
    }

    return munged;
  }

  /* TODO: Support symbols and other data; Clojure takes in anything and passes it through str. */
  object_ptr munge(object_ptr o)
  {
    return visit_object(
      [](auto const typed_o) -> object_ptr {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(std::same_as<T, obj::string>)
        {
          return jank::make_box<obj::string>(munge(typed_o->data));
        }
        else
        {
          throw "munging only supported for strings right now";
        }
      },
      o);
  }
}
