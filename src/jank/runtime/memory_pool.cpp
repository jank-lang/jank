#include <jank/runtime/memory_pool.hpp>
#include <jank/runtime/type/number.hpp>
#include <jank/runtime/type/string.hpp>
#include <jank/runtime/type/vector.hpp>
#include <jank/runtime/type/list.hpp>
#include <jank/runtime/type/map.hpp>
#include <jank/runtime/type/set.hpp>

namespace jank::runtime
{
  template <>
  pool<type::integer>& get_pool<type::integer>()
  {
    static pool<type::integer> p{ 16 };
    return p;
  }
  template <>
  pool<type::real>& get_pool<type::real>()
  {
    static pool<type::real> p{ 16 };
    return p;
  }
  template <>
  pool<type::string>& get_pool<type::string>()
  {
    static pool<type::string> p{ 16 };
    return p;
  }
  template <>
  pool<type::vector>& get_pool<type::vector>()
  {
    static pool<type::vector> p{ 16 };
    return p;
  }
  template <>
  pool<type::map>& get_pool<type::map>()
  {
    static pool<type::map> p{ 16 };
    return p;
  }
  template <>
  pool<type::set>& get_pool<type::set>()
  {
    static pool<type::set> p{ 16 };
    return p;
  }
}
