#pragma once

#include <map>
#include <stdexcept>
#include <experimental/optional>

#include <jank/translate/environment/special/func.hpp>
#include <jank/translate/expect/type.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    {
      namespace special
      {
        inline std::experimental::optional<cell::cell> handle
        (parse::cell::list const &list, cell::cell const &translated)
        {
          static std::map
          <
            std::string,
            std::function
            <cell::cell (parse::cell::list const &input, cell::cell output)>
          > specials
          {
            { "func", &func }
          };

          auto &data(list.data);
          if(data.empty())
          { throw std::runtime_error{ "invalid parse list" }; }

          auto const it(specials.find(expect::type<parse::cell::type::ident>(list.data[0]).data));
          if(it != specials.end())
          { return { it->second(list, translated) }; }
          return {};
        }
      }
    }
  }
}
