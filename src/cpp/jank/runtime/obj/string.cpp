#include <iostream>
#include <sstream>

#include <jank/runtime/seq.hpp>
#include <jank/runtime/util.hpp>
#include <jank/runtime/hash.hpp>
#include <jank/runtime/obj/function.hpp>
#include <jank/runtime/obj/string.hpp>

namespace jank::runtime::obj
{
  string::string(runtime::detail::string_type const &d)
    : data{ d }
  { }
  string::string(runtime::detail::string_type &&d)
    : data{ std::move(d) }
  { }

  runtime::detail::boolean_type string::equal(object const &o) const
  {
    auto const *s(o.as_string());
    if(!s)
    { return false; }

    return data == s->data;
  }
  runtime::detail::string_type string::to_string() const
  { return data; }
  runtime::detail::integer_type string::to_hash() const
  { return data.to_hash(); }
  string const* string::as_string() const
  { return this; }
}
