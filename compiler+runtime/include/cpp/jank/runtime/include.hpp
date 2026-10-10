#include <jtl/immutable_string.hpp>

namespace jank::runtime
{
  namespace obj
  {
    using persistent_list_ref = oref<struct persistent_list>;
  }

  obj::persistent_list_ref include_header(jtl::immutable_string const &header);
  void load_header(jtl::immutable_string const &header);
}
