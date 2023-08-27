#include <iostream>
#include <sstream>

#include <fmt/compile.h>

#include <jank/runtime/util.hpp>
#include <jank/runtime/hash.hpp>
#include <jank/runtime/obj/string.hpp>

namespace jank::runtime
{
  obj::string::static_object(native_string const &d)
    : data{ d }
  { }

  obj::string::static_object(native_string &&d)
    : data{ std::move(d) }
  { }

  native_bool obj::string::equal(object const &o) const
  {
    if(o.type != object_type::string)
    { return false; }

    auto const s(expect_object<obj::string>(&o));
    return data == s->data;
  }

  native_string const& obj::string::to_string() const
  { return data; }

  void obj::string::to_string(fmt::memory_buffer &buff) const
  { format_to(std::back_inserter(buff), FMT_COMPILE("{}"), data); }

  native_integer obj::string::to_hash() const
  { return data.to_hash(); }

  size_t obj::string::count() const
  { return data.size(); }
}
