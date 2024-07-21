#include <iostream>
#include <sstream>

#include <fmt/compile.h>

#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/util/escape.hpp>

namespace jank::runtime
{
  obj::persistent_string::static_object(native_persistent_string const &d)
    : data{ d }
  {
  }

  obj::persistent_string::static_object(native_persistent_string &&d)
    : data{ std::move(d) }
  {
  }

  native_bool obj::persistent_string::equal(object const &o) const
  {
    if(o.type != object_type::persistent_string)
    {
      return false;
    }

    auto const s(expect_object<obj::persistent_string>(&o));
    return data == s->data;
  }

  native_persistent_string const &obj::persistent_string::to_string() const
  {
    return data;
  }

  void obj::persistent_string::to_string(fmt::memory_buffer &buff) const
  {
    fmt::format_to(std::back_inserter(buff), FMT_COMPILE("{}"), data);
  }

  native_hash obj::persistent_string::to_hash() const
  {
    return data.to_hash();
  }

  native_integer obj::persistent_string::compare(object const &o) const
  {
    return visit_type<obj::persistent_string>(
      [this](auto const typed_o) { return compare(*typed_o); },
      &o);
  }

  native_integer obj::persistent_string::compare(obj::persistent_string const &s) const
  {
    return data.compare(s.data);
  }

  result<obj::persistent_string_ptr, native_persistent_string>
  obj::persistent_string::substring(native_integer start) const
  {
    return substring(start, static_cast<native_integer>(data.size()));
  }

  result<obj::persistent_string_ptr, native_persistent_string>
  obj::persistent_string::substring(native_integer const start, native_integer const end) const
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

  native_integer obj::persistent_string::first_index_of(object_ptr const m) const
  {
    auto const s(runtime::to_string(m));
    auto const found(data.find(s));
    if(found == native_persistent_string::npos)
    {
      return -1;
    }
    return static_cast<native_integer>(found);
  }

  native_integer obj::persistent_string::last_index_of(object_ptr const m) const
  {
    auto const s(runtime::to_string(m));
    auto const found(data.rfind(s));
    if(found == native_persistent_string::npos)
    {
      return -1;
    }
    return static_cast<native_integer>(found);
  }

  size_t obj::persistent_string::count() const
  {
    return data.size();
  }

  obj::persistent_string_sequence_ptr obj::persistent_string::seq() const
  {
    if(data.empty())
    {
      return nullptr;
    }
    return make_box<obj::persistent_string_sequence>(const_cast<obj::persistent_string *>(this));
  }

  obj::persistent_string_sequence_ptr obj::persistent_string::fresh_seq() const
  {
    if(data.empty())
    {
      return nullptr;
    }
    return make_box<obj::persistent_string_sequence>(const_cast<obj::persistent_string *>(this));
  }
}
