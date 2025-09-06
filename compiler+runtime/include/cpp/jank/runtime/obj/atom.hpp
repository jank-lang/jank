#pragma once

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using atom_ref = oref<struct atom>;
  using persistent_vector_ref = oref<struct persistent_vector>;

  struct atom : object
  {
    static constexpr object_type obj_type{ object_type::atom };
    static constexpr bool pointer_free{ false };

    atom();
    atom(object_ref o);

    /* behavior::object_like */
    bool equal(object const &) const override;
    jtl::immutable_string to_string() const override;
    void to_string(jtl::string_builder &buff) const override;
    jtl::immutable_string to_code_string() const override;
    uhash to_hash() const override;

    /* behavior::derefable */
    object_ref deref() const;

    /* Replaces the old value with the specified value. Returns the new value. */
    object_ref reset(object_ref o);
    /* Same as reset, but returns a vector of the old value and the new value. */
    persistent_vector_ref reset_vals(object_ref o);

    /* Atomically updates the value of the atom with the specified fn. Returns the new value. */
    object_ref swap(object_ref fn);
    object_ref swap(object_ref fn, object_ref a1);
    object_ref swap(object_ref fn, object_ref a1, object_ref a2);
    object_ref swap(object_ref fn, object_ref a1, object_ref a2, object_ref rest);

    /* Same as swap, but returns a vector of the old value and the new value. */
    persistent_vector_ref swap_vals(object_ref fn);
    persistent_vector_ref swap_vals(object_ref fn, object_ref a1);
    persistent_vector_ref swap_vals(object_ref fn, object_ref a1, object_ref a2);
    persistent_vector_ref swap_vals(object_ref fn, object_ref a1, object_ref a2, object_ref rest);

    object_ref compare_and_set(object_ref old_val, object_ref new_val);

    object base{ obj_type };
    std::atomic<object *> val{};
  };
}
