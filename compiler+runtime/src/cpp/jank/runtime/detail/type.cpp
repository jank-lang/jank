#include <jank/runtime/detail/type.hpp>
#include <jank/runtime/core/equal.hpp>

namespace jank::runtime::detail
{
  native_bool object_ptr_compare::operator()(object_ptr const l, object_ptr const r) const
  {
    return runtime::compare(l, r) < 0;
  }
}

namespace immer
{
  template class vector<jank::runtime::object_ptr, jank::memory_policy>;
  template class set<jank::runtime::object_ptr,
                     std::hash<jank::runtime::object_ptr>,
                     std::equal_to<jank::runtime::object_ptr>,
                     jank::memory_policy>;
  template class immer::map<jank::runtime::object_ptr,
                            jank::runtime::object_ptr,
                            std::hash<jank::runtime::object_ptr>,
                            std::equal_to<jank::runtime::object_ptr>,
                            jank::memory_policy>;
}

namespace bpptree::detail
{
  template struct BppTreeSet<jank::runtime::object_ptr, jank::runtime::detail::object_ptr_compare>;
  template struct BppTreeMap<jank::runtime::object_ptr,
                             jank::runtime::object_ptr,
                             jank::runtime::detail::object_ptr_compare>;
}
