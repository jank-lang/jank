#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <jank/runtime/obj/uuid.hpp>
#include <jank/runtime/rtti.hpp>

namespace jank::runtime::obj
{
  uuid::uuid()
  {
    boost::uuids::random_generator gen;
    value = gen();
  }

  uuid::uuid(native_persistent_string const &s)
  {
    value = boost::lexical_cast<boost::uuids::uuid>(s);
  }

  native_bool uuid::equal(object const &o) const
  {
    if(o.type != object_type::uuid)
    {
      return false;
    }

    auto const s(expect_object<uuid>(&o));
    return s->value == value;
  }

  static void to_string_impl(boost::uuids::uuid value, util::string_builder &buff)
  {
    buff("#uuid \"");
    buff(boost::lexical_cast<std::string>(value));
    buff('"');
  }

  void uuid::to_string(util::string_builder &buff) const
  {
    to_string_impl(value, buff);
  }

  native_persistent_string uuid::to_string() const
  {
    util::string_builder buff;
    to_string_impl(value, buff);
    return buff.release();
  }

  native_persistent_string uuid::to_code_string() const
  {
    return to_string();
  }

  native_hash uuid::to_hash() const
  {
    if(hash)
    {
      return hash;
    }

    return hash = boost::hash<boost::uuids::uuid>()(value);
  }
}
