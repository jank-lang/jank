#include <iostream>
#include <sstream>

#include <fmt/compile.h>

#include <jank/runtime/util.hpp>
#include <jank/runtime/hash.hpp>
#include <jank/runtime/obj/function.hpp>
#include <jank/runtime/obj/string.hpp>

namespace jank::runtime::obj
{
  string::string(native_string const &d)
    : data{ d }
  { }
  string::string(native_string &&d)
    : data{ std::move(d) }
  { }

  native_bool string::equal(object const &o) const
  {
    auto const *s(o.as_string());
    if(!s)
    { return false; }

    return data == s->data;
  }
  native_string string::to_string() const
  { return data; }
  void string::to_string(fmt::memory_buffer &buff) const
  { format_to(std::back_inserter(buff), FMT_COMPILE("{}"), data); }
  native_integer string::to_hash() const
  {
    static std::hash<native_string> hasher{};
    return hasher(data);
  }
  string const* string::as_string() const
  { return this; }

  size_t string::count() const
  { return data.size(); }

  object_ptr string::with_meta(object_ptr const m) const
  {
    auto const meta(validate_meta(m));
    auto * const ret(jank::make_box<string>(data));
    ret->meta = meta;
    return ret;
  }

  behavior::metadatable const* string::as_metadatable() const
  { return this; }
}
