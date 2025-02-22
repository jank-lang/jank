#include <fmt/format.h>

#include <jank/native_persistent_string/fmt.hpp>
#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/obj/persistent_string_sequence.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/util/escape.hpp>

namespace jank::runtime::obj
{
  persistent_string::persistent_string(native_persistent_string const &d)
    : data{ d }
  {
  }

  persistent_string::persistent_string(native_persistent_string &&d)
    : data{ std::move(d) }
  {
  }

  native_bool persistent_string::equal(object const &o) const
  {
    if(o.type != object_type::persistent_string)
    {
      return false;
    }

    auto const s(expect_object<persistent_string>(&o));
    return data == s->data;
  }

  native_persistent_string const &persistent_string::to_string() const
  {
    return data;
  }

  void persistent_string::to_string(util::string_builder &buff) const
  {
    buff(data);
  }

  native_persistent_string persistent_string::to_code_string() const
  {
    util::string_builder sb;
    return sb('"')(util::escape(data))('"').release();
  }

  native_hash persistent_string::to_hash() const
  {
    return data.to_hash();
  }

  native_integer persistent_string::compare(object const &o) const
  {
    return compare(*try_object<persistent_string>(&o));
  }

  native_integer persistent_string::compare(persistent_string const &s) const
  {
    return data.compare(s.data);
  }

  object_ptr persistent_string::get(object_ptr const key) const
  {
    return get(key, nil::nil_const());
  }

  object_ptr persistent_string::get(object_ptr const key, object_ptr const fallback) const
  {
    if(key->type == object_type::integer)
    {
      auto const i(expect_object<integer>(key)->data);
      if(i < 0 || data.size() <= static_cast<size_t>(i))
      {
        return fallback;
      }
      return make_box<character>(data[i]);
    }
    else
    {
      return fallback;
    }
  }

  native_bool persistent_string::contains(object_ptr const key) const
  {
    if(key->type == object_type::integer)
    {
      auto const i(expect_object<integer>(key)->data);
      return 0 <= i && static_cast<size_t>(i) < data.size();
    }
    return false;
  }

  object_ptr persistent_string::get_entry(object_ptr const) const
  {
    throw std::runtime_error{ fmt::format("get_entry not supported on string") };
  }

  string_result<persistent_string_ptr> persistent_string::substring(native_integer start) const
  {
    return substring(start, static_cast<native_integer>(data.size()));
  }

  string_result<persistent_string_ptr>
  persistent_string::substring(native_integer const start, native_integer const end) const
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

  native_integer persistent_string::first_index_of(object_ptr const m) const
  {
    auto const s(runtime::to_string(m));
    auto const found(data.find(s));
    if(found == native_persistent_string::npos)
    {
      return -1;
    }
    return static_cast<native_integer>(found);
  }

  native_integer persistent_string::last_index_of(object_ptr const m) const
  {
    auto const s(runtime::to_string(m));
    auto const found(data.rfind(s));
    if(found == native_persistent_string::npos)
    {
      return -1;
    }
    return static_cast<native_integer>(found);
  }

  size_t persistent_string::count() const
  {
    return data.size();
  }

  persistent_string_sequence_ptr persistent_string::seq() const
  {
    return fresh_seq();
  }

  persistent_string_sequence_ptr persistent_string::fresh_seq() const
  {
    if(data.empty())
    {
      return nullptr;
    }
    return make_box<persistent_string_sequence>(const_cast<persistent_string *>(this));
  }
}
