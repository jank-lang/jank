#pragma once

#include <jank/runtime/memory_pool.hpp>
#include <jank/runtime/detail/type.hpp>

namespace jank::runtime
{
  using object_ptr = detail::box_type<struct object>;

  namespace behavior
  {
    struct callable : virtual pool_item_common_base
    {
      virtual object_ptr call() const;
      virtual object_ptr call(object_ptr const&) const;
      virtual object_ptr call(object_ptr const&, object_ptr const&) const;
      virtual object_ptr call(object_ptr const&, object_ptr const&, object_ptr const&) const;
      virtual object_ptr call(object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&) const;
      virtual object_ptr call(object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&) const;
      virtual object_ptr call(object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&) const;
      virtual object_ptr call(object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&) const;
      virtual object_ptr call(object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&) const;
      virtual object_ptr call(object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&) const;
      virtual object_ptr call(object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&, object_ptr const&) const;
    };
  }
}
