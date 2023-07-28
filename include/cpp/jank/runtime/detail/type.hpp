#pragma once

#include <immer/vector.hpp>
#include <immer/vector_transient.hpp>
#include <immer/map.hpp>
#include <immer/map_transient.hpp>
#include <immer/set.hpp>
#include <immer/set_transient.hpp>
#include <immer/memory_policy.hpp>

#include <jank/runtime/object.hpp>
#include <jank/runtime/detail/list_type.hpp>

namespace jank::runtime::detail
{
  using persistent_list = list_type_impl<object_ptr>;
  using peristent_vector = immer::vector<object_ptr, memory_policy>;
  using transient_vector = peristent_vector::transient_type;
  using persistent_set = immer::set<object_ptr, std::hash<object_ptr>, std::equal_to<>, memory_policy>;
  using transient_set = persistent_set::transient_type;
  //using persistent_map = immer::map<object_ptr, object_ptr, std::hash<object_ptr>, object_ptr_equal, detail::memory_policy>;
  //using persistent_map = std::map<object_ptr, object_ptr, object_ptr_less>;
  //using transient_map = map_type::transient_type;
}
