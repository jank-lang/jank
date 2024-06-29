#include <jank/runtime/obj/character.hpp>
#include <jank/util/character.hpp>

namespace jank::runtime
{
  obj::character::static_object(native_persistent_string_view const &d)
    : data{ native_persistent_string(d) }
  {
  }

  native_bool obj::character::equal(object const &o) const
  {
    if(o.type != object_type::character)
    {
      return false;
    }

    auto const c(expect_object<obj::character>(&o));
    return data == c->data;
  }

  void obj::character::to_string(fmt::memory_buffer &buff) const
  {
    fmt::format_to(std::back_inserter(buff), "{}", data);
  }

  native_persistent_string obj::character::to_string() const
  {
    fmt::memory_buffer buff;
    to_string(buff);
    return native_persistent_string{ buff.data(), buff.size() };
  }

  native_hash obj::character::to_hash() const
  {
    return static_cast<native_hash>(util::character::get_char_from_repr(data).unwrap());
  }
}
