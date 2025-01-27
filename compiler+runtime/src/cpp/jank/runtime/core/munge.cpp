#include <regex>

#include <jank/runtime/core/munge.hpp>
#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/rtti.hpp>

namespace jank::runtime
{
  static native_unordered_map<char, native_persistent_string_view> const munge_chars{
    {  '-',        "_" },
    {  ':',  "_COLON_" },
    {  '+',   "_PLUS_" },
    {  '>',     "_GT_" },
    {  '<',     "_LT_" },
    {  '=',     "_EQ_" },
    {  '~',  "_TILDE_" },
    {  '!',   "_BANG_" },
    {  '@',  "_CIRCA_" },
    {  '#',  "_SHARP_" },
    { '\'', "_SQUOTE_" },
    {  '"', "_DQUOTE_" },
    {  '%',   "_PERC_" },
    {  '^',  "_CARET_" },
    {  '&',    "_AMP_" },
    {  '*',   "_STAR_" },
    {  '|',    "_BAR_" },
    {  '{', "_LBRACE_" },
    {  '}', "_RBRACE_" },
    {  '[', "_LBRACK_" },
    {  ']', "_RBRACK_" },
    {  '/',  "_SLASH_" },
    { '\\', "_BSLASH_" },
    {  '?',  "_QMARK_" }
  };

  /* This is sorted from longest to shortest so we can check for the longest first. This
   * allows some entries to be prefixes of others without ambiguity. */
  static native_vector<std::pair<native_persistent_string_view, char>> const demunge_chars{
    { "_LBRACE_",  '{' },
    { "_RBRACE_",  '}' },
    { "_LBRACK_",  '[' },
    { "_RBRACK_",  ']' },
    { "_BSLASH_", '\\' },
    { "_SQUOTE_", '\'' },
    { "_DQUOTE_",  '"' },
    {  "_QMARK_",  '?' },
    {  "_COLON_",  ':' },
    {  "_TILDE_",  '~' },
    {  "_CIRCA_",  '@' },
    {  "_SHARP_",  '#' },
    {  "_CARET_",  '^' },
    {  "_SLASH_",  '/' },
    {   "_PERC_",  '%' },
    {   "_PLUS_",  '+' },
    {   "_BANG_",  '!' },
    {   "_STAR_",  '*' },
    {    "_AMP_",  '&' },
    {    "_BAR_",  '|' },
    {     "_GT_",  '>' },
    {     "_LT_",  '<' },
    {     "_EQ_",  '=' },
    {        "_",  '-' },
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

  static native_set<native_persistent_string_view> const munged_cpp_keywords{
    "alignas__",
    "alignof__",
    "and__",
    "and_eq__",
    "asm__",
    "atomic_cancel__",
    "atomic_commit__",
    "atomic_noexcept__",
    "auto__",
    "bitand__",
    "bitor__",
    "bool__",
    "break__",
    "case__",
    "catch__",
    "char__",
    "char8_t__",
    "char16_t__",
    "char32_t__",
    "class__",
    "compl__",
    "concept__",
    "const__",
    "consteval__",
    "constexpr__",
    "constinit__",
    "const_cast__",
    "continue__",
    "co_await__",
    "co_return__",
    "co_yield__",
    "decltype__",
    "default__",
    "delete__",
    "do__",
    "double__",
    "dynamic_cast__",
    "else__",
    "enum__",
    "explicit__",
    "export__",
    "extern__",
    "false__",
    "float__",
    "for__",
    "friend__",
    "goto__",
    "if__",
    "inline__",
    "int__",
    "long__",
    "mutable__",
    "namespace__",
    "new__",
    "noexcept__",
    "not__",
    "not_eq__",
    "nullptr__",
    "operator__",
    "or__",
    "or_eq__",
    "private__",
    "protected__",
    "public__",
    "reflexpr__",
    "register__",
    "reinterpret_cast__",
    "requires__",
    "return__",
    "short__",
    "signed__",
    "sizeof__",
    "static__",
    "static_assert__",
    "static_cast__",
    "struct__",
    "switch__",
    "synchronized__",
    "template__",
    "this__",
    "thread_local__",
    "throw__",
    "true__",
    "try__",
    "typedef__",
    "typeid__",
    "typename__",
    "union__",
    "unsigned__",
    "using__",
    "virtual__",
    "void__",
    "volatile__",
    "wchar_t__",
    "while__",
    "xor__",
    "xor_eq__",
  };

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

  native_persistent_string munge_namespace(native_persistent_string const &o)
  {
    static std::regex const dash{ "-" };
    native_transient_string const ret{ o };
    return std::regex_replace(ret, dash, "_");
  }

  native_persistent_string munge_extra(native_persistent_string const &o,
                                       native_persistent_string const &search,
                                       char const * const replace)
  {
    native_transient_string const ret{ munge(o) };
    std::regex const search_regex{ search.c_str() };
    return std::regex_replace(ret, search_regex, replace);
  }

  /* TODO: Support symbols and other data; Clojure takes in anything and passes it through str. */
  object_ptr munge(object_ptr const o)
  {
    if(auto const s = dyn_cast<obj::persistent_string>(o))
    {
      return make_box<obj::persistent_string>(munge(s->data));
    }
    else
    {
      throw std::runtime_error{ "munging only supported for strings right now" };
    }
  }

  native_persistent_string demunge(native_persistent_string const &o)
  {
    if(munged_cpp_keywords.contains(o))
    {
      /* Remove the __ suffix. */
      return o.substr(0, o.size() - 2);
    }

    native_transient_string ret{ o };

    for(auto const &pair : demunge_chars)
    {
      size_t pos{};
      auto const pattern_length{ pair.first.length() };
      native_transient_string tmp;
      tmp.reserve(ret.size());

      while(true)
      {
        auto const found{ ret.find(pair.first, pos) };
        if(found == native_transient_string::npos)
        {
          tmp.append(ret, pos, ret.size() - pos);
          break;
        }
        tmp.append(ret, pos, found - pos);
        tmp += pair.second;
        pos = found + pattern_length;
      }

      ret = tmp;
    }

    return ret;
  }
}
