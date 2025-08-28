#include <uuid.h>
#include <jank/runtime/obj/uuid.hpp>
#include <jank/runtime/rtti.hpp>

namespace jank::runtime::obj
{
  uuid::uuid()
  {
    static std::random_device rd;
    std::mt19937 g(rd());
    uuids::uuid_random_generator gen{ g };
    value = gen();
  }

  uuid::uuid(jtl::immutable_string const &s)
  {
    value = uuids::uuid::from_string(s.c_str()).value();
  }

  bool uuid::equal(object const &o) const
  {
    if(o.type != object_type::uuid)
    {
      return false;
    }

    auto const s(expect_object<uuid>(&o));
    return s->value == value;
  }

  void uuid::to_string(jtl::string_builder &buff) const
  {
    buff(uuids::to_string(value));
  }

  jtl::immutable_string uuid::to_string() const
  {
    jtl::string_builder buff;
    buff(uuids::to_string(value));
    return buff.release();
  }

  jtl::immutable_string uuid::to_code_string() const
  {
    jtl::string_builder buff;
    buff("#uuid \"");
    buff(uuids::to_string(value));
    buff('"');
    return buff.release();
  }

  uhash uuid::to_hash() const
  {
    if(hash)
    {
      return hash;
    }
    static std::hash<uuids::uuid> to_hash{};
    return hash = static_cast<uhash>(to_hash(value));
  }
}
