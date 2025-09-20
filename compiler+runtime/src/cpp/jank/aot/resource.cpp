#include <folly/Synchronized.h>

#include <jank/aot/resource.hpp>

namespace jank::aot
{
  static folly::Synchronized<
    native_unordered_map<jtl::immutable_string, jtl::immutable_string_view>>
    /* NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) */
    mapped_resources;

  void register_resource(jtl::immutable_string const &name, jtl::immutable_string_view const &data)
  {
    auto locked_resources{ mapped_resources.wlock() };
    (*locked_resources).insert_or_assign(name, data);
  }

  jtl::option<jtl::immutable_string_view> find_resource(jtl::immutable_string const &name)
  {
    auto locked_resources{ mapped_resources.rlock() };
    auto const found{ locked_resources->find(name) };
    if(found == locked_resources->end())
    {
      return none;
    }
    return found->second;
  }
}
