#pragma once

#include <ostream>

#include <jtl/iterator/range.hpp>

namespace jank
{
  namespace detail
  {
    namespace stream
    {
      struct indenter
      {
        indenter(std::ostream &os, int &level)
          : level{ level }
        {
          ++level;
          os << "\n";
          for(auto const i : jtl::it::make_range(0, level))
          { static_cast<void>(i); os << "  "; }
        }
        ~indenter()
        { --level; }

        int &level;
      };
    }
  }
}
