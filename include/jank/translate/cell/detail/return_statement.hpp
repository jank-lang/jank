#pragma once

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      namespace detail
      {
        /* C = cell, T = cell::type. */
        template <typename C, typename T>
        struct return_statement
        {
          C cell;
          T expected_type;
        };
      }
    }
  }
}
