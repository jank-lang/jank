#include <unordered_map>

#include <jank/runtime/util.hpp>
#include <jank/runtime/obj/string.hpp>

namespace jank::runtime
{
  namespace detail
  {
    bool truthy(object_ptr o)
    {
      if(o->as_nil())
      { return false; }
      else if(auto const * const b = o->as_boolean())
      { return b->data; }

      return true;
    }
  }

  object_ptr identity(object_ptr o)
  { return o; }

  /* some? */
  object_ptr some_gen_qmark_(object_ptr o)
  { return make_box<obj::boolean>(o->as_nil() == nullptr); }

  /* nil? */
  object_ptr nil_gen_qmark_(object_ptr o)
  { return make_box<obj::boolean>(o->as_nil() != nullptr); }

  /* truthy? */
  object_ptr truthy_gen_qmark_(object_ptr o)
  { return make_box<obj::boolean>(detail::truthy(o)); }

  /* = */
  object_ptr _gen_equal_(object_ptr l, object_ptr r)
  { return make_box<obj::boolean>(l->equal(*r)); }

  /* not= */
  object_ptr not_gen_equal_(object_ptr l, object_ptr r)
  { return make_box<obj::boolean>(!l->equal(*r)); }

  /* TODO: This should be the `and` macro. */
  object_ptr all(object_ptr l, object_ptr r)
  { return make_box<obj::boolean>(detail::truthy(l) && detail::truthy(r));}

  /* TODO: This should be the `or` macro. */
  object_ptr either(object_ptr l, object_ptr r)
  { return detail::truthy(l) ? l : r;}

  static std::unordered_map<char, std::string_view> const munge_chars
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

  detail::string_type munge(detail::string_type const &o)
  {
    std::string munged;
    for(auto const &c : o.data)
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
    auto const * const str(o->as_string());
    if(str == nullptr)
    { throw "munging only supported for strings right now"; }

    return make_box<obj::string>(munge(str->data));
  }
}
