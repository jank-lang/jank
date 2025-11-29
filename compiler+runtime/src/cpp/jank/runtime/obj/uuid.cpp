#include <uuid.h>

#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/obj/uuid.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::runtime::obj
{
  static jtl::ref<uuids::uuid> random()
  {
    static std::random_device rd;
    std::mt19937 g(rd());
    uuids::uuid_random_generator gen{ g };
    return jtl::make_ref<uuids::uuid>(gen());
  }

  static jtl::ref<uuids::uuid> from_string(jtl::immutable_string const &s)
  {
    auto const result{ uuids::uuid::from_string(s.c_str()) };
    if(result)
    {
      return jtl::make_ref<uuids::uuid>(result.value());
    }
    else
    {
      throw make_box(util::format("Invalid UUID string: {}", s)).erase();
    }
  }

  uuid::uuid()
    : value{ random() }
  {
  }

  uuid::uuid(jtl::immutable_string const &s)
    : value{ from_string(s) }
  {
  }

  bool uuid::equal(object const &o) const
  {
    if(o.type != object_type::uuid)
    {
      return false;
    }

    auto const s(expect_object<uuid>(&o));
    return *s->value == *value;
  }

  void uuid::to_string(jtl::string_builder &buff) const
  {
    buff(uuids::to_string(*value));
  }

  jtl::immutable_string uuid::to_string() const
  {
    jtl::string_builder buff;
    buff(uuids::to_string(*value));
    return buff.release();
  }

  jtl::immutable_string uuid::to_code_string() const
  {
    jtl::string_builder buff;
    buff("#uuid \"");
    buff(uuids::to_string(*value));
    buff('"');
    return buff.release();
  }

  uhash uuid::to_hash() const
  {
    if(hash)
    {
      return hash;
    }
    static std::hash<uuids::uuid> const hasher{};
    return hash = static_cast<uhash>(hasher(*value));
  }
}
