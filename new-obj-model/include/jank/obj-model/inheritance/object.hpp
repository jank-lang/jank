#pragma once

#include <jank/runtime/detail/map_type.hpp>

namespace jank::obj_model::inheritance
{
  struct countable
  {
    virtual ~countable() = default;
    virtual size_t count() const = 0;
  };

  struct seqable
  {
    virtual ~seqable() = default;
    virtual runtime::object_ptr seq() const = 0;
  };

  struct metadatable
  {
    virtual ~metadatable() = default;
    virtual runtime::object_ptr with_meta() const = 0;

    runtime::object_ptr meta{};
  };

  struct associatively_readable
  {
    virtual ~associatively_readable() = default;
    virtual runtime::object_ptr get(runtime::object_ptr key) const = 0;
  };

  struct associatively_writable
  {
    virtual ~associatively_writable() = default;
    virtual runtime::object_ptr assoc(runtime::object_ptr key, runtime::object_ptr val) const = 0;
  };

  //struct map : runtime::object, countable, seqable, metadatable, associatively_readable, associatively_writable
  //{
  //  size_t count() const override
  //  { return 0; }
  //  runtime::object_ptr seq() const override
  //  { return nullptr; }
  //  runtime::object_ptr with_meta() const override
  //  { return nullptr; }
  //  runtime::object_ptr get(runtime::object_ptr) const override
  //  { return nullptr; }
  //  runtime::object_ptr assoc(runtime::object_ptr, runtime::object_ptr) const override
  //  { return nullptr; }

  //  native_string to_string() const final
  //  { return ""; }
  //  void to_string(fmt::memory_buffer &) const final
  //  { }
  //  native_integer to_hash() const final
  //  { return 0; }

  //  runtime::detail::persistent_map data{};
  //};
  using map = runtime::obj::map;
}
