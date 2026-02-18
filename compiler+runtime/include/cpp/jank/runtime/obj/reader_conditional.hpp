#pragma once

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/type.hpp>

namespace jank::runtime::obj
{
  using reader_conditional_ref = oref<struct reader_conditional>;

  struct reader_conditional : object
  {
    using value_type = runtime::detail::native_persistent_vector;

    static constexpr object_type obj_type{ object_type::reader_conditional };
    static constexpr object_behavior obj_behaviors{ object_behavior::none };
    static constexpr bool pointer_free{ false };

    reader_conditional();
    reader_conditional(reader_conditional &&) noexcept = default;
    reader_conditional(reader_conditional const &) = default;
    reader_conditional(value_type &&d);
    reader_conditional(value_type const &d);

    /* behavior::object_like */
    bool equal(object const &) const override;
    jtl::immutable_string to_string() const override;
    void to_string(jtl::string_builder &buff) const override;
    jtl::immutable_string to_code_string() const override;
    uhash to_hash() const override;

    /*** XXX: Everything here is immutable after initialization. ***/
    value_type data;
  };
}
