#pragma once

#include <jank/runtime/object.hpp>
#include <future>

namespace jank::runtime::obj
{
  using future_ptr = native_box<struct future>;

  struct future : gc
  {
    static constexpr object_type obj_type{ object_type::future };
    static constexpr native_bool pointer_free{ false };

    future() = default;
    future(object_ptr fn);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    native_persistent_string to_string() const;
    void to_string(util::string_builder &buff) const;
    native_persistent_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::derefable */
    object_ptr deref();

    /* behavior::blocking_derefable */
    object_ptr blocking_deref(object_ptr const, object_ptr const);

    object base{ obj_type };
    std::shared_future<object_ptr> fut;
  };
}
