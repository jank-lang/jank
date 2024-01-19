#include <iostream>
#include <sstream>

#include <fmt/compile.h>

#include <jank/runtime/util.hpp>
#include <jank/runtime/obj/string.hpp>

namespace jank::runtime
{
  obj::string::static_object(native_persistent_string const &d)
    : data{ d }
  {
  }

  obj::string::static_object(native_persistent_string &&d)
    : data{ std::move(d) }
  {
  }

  native_bool obj::string::equal(object const &o) const
  {
    if(o.type != object_type::string)
    {
      return false;
    }

    auto const s(expect_object<obj::string>(&o));
    return data == s->data;
  }

  native_persistent_string const &obj::string::to_string() const
  {
    return data;
  }

  void obj::string::to_string(fmt::memory_buffer &buff) const
  {
    format_to(std::back_inserter(buff), FMT_COMPILE("{}"), data);
  }

  native_hash obj::string::to_hash() const
  {
    return data.to_hash();
  }

  result<obj::string_ptr, native_persistent_string>
  obj::string::substring(native_integer start) const
  {
    return substring(start, static_cast<native_integer>(data.size()));
  }

  result<obj::string_ptr, native_persistent_string>
  obj::string::substring(native_integer const start, native_integer const end) const
  {
    if(start < 0)
    {
      return err(fmt::format("start index {} is less than 0", start));
    }
    if(end < 0)
    {
      return err(fmt::format("end index {} is less than 0", start));
    }
    else if(static_cast<size_t>(start) > data.size())
    {
      return err(fmt::format("start index {} is outside the bounds of {}", start, data.size()));
    }
    else if(static_cast<size_t>(end) > data.size())
    {
      return err(fmt::format("end index {} is outside the bounds of {}", end, data.size()));
    }

    return ok(make_box(data.substr(start, end)));
  }

  native_integer obj::string::first_index_of(object_ptr const c) const
  {
    auto const s(runtime::detail::to_string(c));
    auto const found(data.find(s));
    if(found == native_persistent_string::npos)
    {
      return -1;
    }
    return static_cast<native_integer>(found);
  }

  native_integer obj::string::last_index_of(object_ptr const c) const
  {
    auto const s(runtime::detail::to_string(c));
    auto const found(data.rfind(s));
    if(found == native_persistent_string::npos)
    {
      return -1;
    }
    return static_cast<native_integer>(found);
  }

  size_t obj::string::count() const
  {
    return data.size();
  }
}
