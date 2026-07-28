#pragma once

#include <jtl/option.hpp>

#include <jank/runtime/object.hpp>
#include <jank/runtime/lazy_meta.hpp>

namespace jank::runtime::obj
{
  using cons_ref = oref<struct cons>;
  using repeat_ref = oref<struct repeat>;

  struct repeat : object
  {
    static constexpr object_type obj_type{ object_type::repeat };
    static constexpr object_behavior obj_behaviors{ object_behavior::seqable
                                                    | object_behavior::fresh_seqable
                                                    | object_behavior::sequence_like
                                                    | object_behavior::sequence_like_in_place };
    static constexpr bool pointer_free{ false };
    static constexpr bool is_sequential{ true };
    static constexpr i64 infinite{ -1 };

    repeat();
    repeat(object_ref const value);
    repeat(i64 const count, object_ref const value);

    static object_ref create(object_ref const value);
    static object_ref create(i64 const count, object_ref const value);

    /* behavior::object_like */
    bool equal(object const &) const override;
    jtl::immutable_string to_string() const override;
    void to_string(jtl::string_builder &buff) const override;
    jtl::immutable_string to_code_string() const override;
    uhash to_hash() const override;

    /* behavior::seqable */
    object_ref seq() const override;

    /* behavior::fresh_seqable */
    object_ref fresh_seq() const override;

    /* behavior::sequence_like */
    object_ref first() const override;
    object_ref next() const override;

    /* behavior::sequence_like_in_place */
    object_ref next_in_place() override;

    /* behavior::conjable */
    obj::cons_ref conj(object_ref const head) const;

    /* behavior::metadatable */
    repeat_ref with_meta(object_ref const m) const;
    object_ref get_meta() const;
    void set_meta(object_ref const o);

    /*** XXX: Everything here is immutable after initialization. ***/
    object_ref value{};
    i64 count{};

  private:
    /*** XXX: Everything here is thread-safe. ***/
    lazy_meta meta{};
  };
}
