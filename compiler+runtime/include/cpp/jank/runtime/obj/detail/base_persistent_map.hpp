#pragma once

#include <jank/runtime/object.hpp>
#include <jank/option.hpp>

namespace jank::runtime
{
  native_bool is_map(object_ptr o);
  native_bool equal(object_ptr l, object_ptr r);
  void to_string(object_ptr o, util::string_builder &buff);
  void to_code_string(object_ptr o, util::string_builder &buff);

  namespace behavior::detail
  {
    object_ptr validate_meta(object_ptr const m);
  }
}

namespace jank::runtime::obj::detail
{
  /* Array maps and hash maps share a lot of common code, so we have a common base.
   * No virtual fns are used, so this structure won't survive release optimizations. */
  template <typename PT, typename ST, typename V>
  struct base_persistent_map : gc
  {
    static constexpr native_bool pointer_free{ false };
    static constexpr native_bool is_map_like{ true };

    using value_type = V;

    base_persistent_map() = default;
    base_persistent_map(option<object_ptr> const &meta);

    /* behavior::object_like */
    native_bool equal(object const &o) const;
    static void to_string_impl(typename V::const_iterator const &begin,
                               typename V::const_iterator const &end,
                               util::string_builder &buff,
                               native_bool const to_code);
    void to_string(util::string_builder &buff) const;

    native_persistent_string to_string() const;
    native_persistent_string to_code_string() const;
    native_hash to_hash() const;

    /* behavior::seqable */
    native_box<ST> seq() const;
    native_box<ST> fresh_seq() const;

    /* behavior::countable */
    size_t count() const;

    /* behavior::metadatable */
    native_box<PT> with_meta(object_ptr const m) const;

    /* behavior::conjable */
    object_ptr conj(object_ptr const head) const;

    object base{ PT::obj_type };
    option<object_ptr> meta;
    mutable native_hash hash{};
  };
}
