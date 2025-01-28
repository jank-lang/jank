#pragma once

#include <boost/uuid/uuid.hpp>

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  struct uuid : gc
  {
    static constexpr object_type obj_type{ object_type::uuid };
    static constexpr native_bool pointer_free{ false };

    uuid();
    uuid(native_persistent_string const &s);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(util::string_builder &buff) const;
    native_persistent_string to_code_string() const;
    native_hash to_hash() const;

    object base{ obj_type };

    boost::uuids::uuid value{};

    mutable native_hash hash{};
  };
}
