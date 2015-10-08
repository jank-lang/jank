#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/environment/builtin/type/macro_primitive.hpp>
#include <jank/translate/macro/emit_state.hpp>
#include <jank/interpret/cell/trait.hpp>
#include <jank/interpret/environment/resolve_value.hpp>
#include <jank/interpret/plugin/detail/make_function.hpp>
#include <jank/interpret/expect/type.hpp>
#include <jank/interpret/expect/error/internal/exception.hpp>

namespace jank
{
  namespace interpret
  {
    namespace plugin
    {
      namespace macro
      {
        /* TODO: Move to separate file. */
        static parse::cell::cell marshal(cell::cell const &input)
        {
          switch(cell::trait::to_enum(input))
          {
            case cell::type::null:
              return parse::cell::null{};
            case cell::type::boolean:
            {
              auto const &c(expect::type<cell::type::boolean>(input));
              return parse::cell::boolean{ c.data };
            }
            case cell::type::integer:
            {
              auto const &c(expect::type<cell::type::integer>(input));
              return parse::cell::integer{ c.data };
            }
            case cell::type::real:
            {
              auto const &c(expect::type<cell::type::real>(input));
              return parse::cell::real{ c.data };
            }
            case cell::type::string:
            {
              auto const &c(expect::type<cell::type::string>(input));
              return parse::cell::string{ c.data };
            }
            case cell::type::list:
            {
              auto const &c(expect::type<cell::type::list>(input));
              parse::cell::list ret;
              std::transform
              (
                c.data.begin(), c.data.end(),
                std::back_inserter(ret.data),
                [](auto const &e)
                { return marshal(e); }
              );
              return ret;
            }
            case cell::type::function:
            default:
              throw expect::error::internal::exception<>
              { "invalid interpret cell while marshalling" };
          }
        }

        void emit
        (
          std::shared_ptr<translate::environment::scope> const &trans_scope,
          std::shared_ptr<environment::scope> const &int_scope
        )
        {
          detail::make_function
          (
            trans_scope, int_scope,
            "emit",
            [](auto const &scope, auto const &args)
            {
              translate::macro::emit
              (marshal(environment::resolve_value(scope, args[0].cell)));
              return cell::null{};
            },
            translate::environment::builtin::type::macro_atom(*trans_scope)
          );
          detail::make_function
          (
            trans_scope, int_scope,
            "emit",
            [](auto const &scope, auto const &args)
            {
              translate::macro::emit
              (marshal(environment::resolve_value(scope, args[0].cell)));
              return cell::null{};
            },
            translate::environment::builtin::type::macro_list(*trans_scope)
          );
        }
      }
    }
  }
}
