#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using native_pointer_wrapper_ref = oref<struct native_pointer_wrapper>;

  struct native_pointer_wrapper : object
  {
    static constexpr object_type obj_type{ object_type::native_pointer_wrapper };
    static constexpr object_behavior obj_behaviors{ object_behavior::none };
    static constexpr bool pointer_free{ false };

    native_pointer_wrapper();
    native_pointer_wrapper(native_pointer_wrapper &&) noexcept = default;
    native_pointer_wrapper(native_pointer_wrapper const &) = default;
    native_pointer_wrapper(void * const);

    /* behavior::object_like */
    bool equal(object const &) const override;

    template <typename T>
    T *as() const
    {
      return reinterpret_cast<T *>(data);
    }

    /*** XXX: Everything here is immutable after initialization. ***/
    void *data{};
  };
}
