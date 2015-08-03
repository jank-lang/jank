#include <jank/translate/environment/special/all.hpp>
#include <jank/parse/expect/type.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    {
      namespace special
      {
        std::experimental::optional<cell::cell> handle
        (
          parse::cell::list const &list,
          cell::function_body const &outer_body
        )
        {
          static std::map
          <
            std::string,
            std::function
            <
              cell::cell
              (
                parse::cell::list const &input,
                cell::function_body const &outer_body
              )
            >
          > specials
          {
            {
              "function",
              [](auto const &input, auto const &body)
              { return function(input, body.data.scope); }
            },
            {
              "ƒ",
              [](auto const &input, auto const &body)
              { return function(input, body.data.scope); }
            },
            {
              "bind",
              [](auto const &input, auto const &body)
              { return binding(input, body.data.scope); }
            },
            {
              "return",
              &return_statement
            },
            {
              "if",
              &if_statement
            },
            {
              "do",
              &do_statement
            },
          };

          auto &data(list.data);
          if(data.empty())
          { throw std::runtime_error{ "invalid parse list" }; }

          auto const it
          (
            specials.find
            (parse::expect::type<parse::cell::type::ident>(list.data[0]).data)
          );
          if(it != specials.end())
          { return { it->second(list, outer_body) }; }
          return {};
        }
      }
    }
  }
}
