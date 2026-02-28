#pragma once

#include <jank/runtime/obj/persistent_list.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/type.hpp>

namespace jank::runtime::obj
{
  using reader_conditional_ref = oref<struct reader_conditional>;

  struct reader_conditional : object
  {
    static constexpr object_type obj_type{ object_type::reader_conditional };
    static constexpr object_behavior obj_behaviors{ object_behavior::get };
    static constexpr bool pointer_free{ false };

    reader_conditional();
    reader_conditional(reader_conditional &&) noexcept = default;
    reader_conditional(reader_conditional const &) = default;
    reader_conditional(persistent_list_ref f, boolean_ref s);

    /* behavior::object_like */
    bool equal(object const &) const override;
    jtl::immutable_string to_string() const override;
    void to_string(jtl::string_builder &buff) const override;
    jtl::immutable_string to_code_string() const override;
    uhash to_hash() const override;

    /* behavior::get */
    object_ref get(object_ref const key) const override;
    object_ref get(object_ref const key, object_ref const fallback) const override;
    bool contains(object_ref const key) const override;

    /*** XXX: Everything here is immutable after initialization. ***/
    persistent_list_ref form{};
    boolean_ref splicing{};
  };
}
