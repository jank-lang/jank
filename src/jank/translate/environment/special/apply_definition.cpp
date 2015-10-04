#include <jank/translate/environment/special/apply_definition.hpp>
#include <jank/parse/expect/type.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    {
      namespace special
      {
        boost::optional<cell::cell> apply_definition
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
              "lambda",
              [](auto const &input, auto const &body)
              { return lambda(input, body.data.scope); }
            },
            {
              "λ",
              [](auto const &input, auto const &body)
              { return lambda(input, body.data.scope); }
            },
            {
              "macro",
              [](auto const &input, auto const &body)
              { return macro(input, body.data.scope); }
            },
          };

          auto &data(list.data);
          if(data.empty()) /* TODO: Turn these all into internal errors. */
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
