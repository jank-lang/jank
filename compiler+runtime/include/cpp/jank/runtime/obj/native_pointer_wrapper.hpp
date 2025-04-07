#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using native_pointer_wrapper_ref = jtl::object_ref<struct native_pointer_wrapper>;

  struct native_pointer_wrapper : gc
  {
    static constexpr object_type obj_type{ object_type::native_pointer_wrapper };
    static constexpr native_bool pointer_free{ false };

    native_pointer_wrapper() = default;
    native_pointer_wrapper(native_pointer_wrapper &&) noexcept = default;
    native_pointer_wrapper(native_pointer_wrapper const &) = default;
    native_pointer_wrapper(void * const);

    /* behavior::object_like */
    native_bool equal(object const &) const;
    jtl::immutable_string to_string() const;
    void to_string(util::string_builder &buff) const;
    jtl::immutable_string to_code_string() const;
    native_hash to_hash() const;

    template <typename T>
    T *as() const
    {
      return reinterpret_cast<T *>(data);
    }

    object base{ obj_type };
    void *data{};
  };
}
