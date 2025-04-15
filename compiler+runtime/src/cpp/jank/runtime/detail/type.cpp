#include <jank/runtime/detail/type.hpp>
#include <jank/runtime/core/equal.hpp>

namespace jank::runtime::detail
{
  native_bool object_ref_compare::operator()(object_ref const l, object_ref const r) const
  {
    return runtime::compare(l, r) < 0;
  }
}

namespace immer
{
  template class vector<jank::runtime::object_ref, jank::memory_policy>;
  template class set<jank::runtime::object_ref,
                     std::hash<jank::runtime::object_ref>,
                     std::equal_to<jank::runtime::object_ref>,
                     jank::memory_policy>;
  template class immer::map<jank::runtime::object_ref,
                            jank::runtime::object_ref,
                            std::hash<jank::runtime::object_ref>,
                            std::equal_to<jank::runtime::object_ref>,
                            jank::memory_policy>;
}

namespace bpptree::detail
{
  template struct BppTreeSet<jank::runtime::object_ref, jank::runtime::detail::object_ref_compare>;
  template struct BppTreeMap<jank::runtime::object_ref,
                             jank::runtime::object_ref,
                             jank::runtime::detail::object_ref_compare>;
}
