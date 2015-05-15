#include <jank/translate/function/argument/resolve_type.hpp>
#include <jank/parse/cell/trait.hpp>
#include <jank/translate/cell/trait.hpp>
#include <jank/translate/expect/type.hpp>
#include <jank/translate/expect/error/type/type.hpp>

namespace jank
{
  namespace translate
  {
    namespace function
    {
      namespace argument
      {
        cell::type_definition resolve_type
        (
          cell::cell const &c,
          std::shared_ptr<environment::scope> const &scope
        )
        {
          switch(cell::trait::to_enum(c))
          {
            case cell::type::variable_reference:
            {
              auto const &ref(expect::type<cell::type::variable_reference>(c));
              return { ref.data.definition.type.definition };
            }
            case cell::type::literal_value:
            {
              auto const &literal(expect::type<cell::type::literal_value>(c));
              auto const name
              (
                parse::cell::trait::to_string
                (static_cast<parse::cell::type>(literal.data.which()))
              );

              auto const &def_opt(scope->find_type(name));
              if(!def_opt)
              {
                throw expect::error::type::type<>
                { std::string{ "invalid literal type: " } + name };
              }

              return def_opt.value();
            }
            default:
              throw expect::error::type::type<>
              { "invalid argument type: " + std::to_string(c.which()) };
          }
        }
      }
    }
  }
}
