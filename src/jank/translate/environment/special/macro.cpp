#include <stdexcept>
#include <memory>

#include <jtl/iterator/range.hpp>

#include <jank/parse/expect/type.hpp>
#include <jank/translate/translate.hpp>
#include <jank/translate/function/argument/definition.hpp>
#include <jank/translate/environment/scope.hpp>
#include <jank/translate/environment/special/macro.hpp>
#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/expect/error/syntax/exception.hpp>
#include <jank/translate/expect/error/type/overload.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    {
      namespace special
      {
        cell::cell macro
        (
          parse::cell::list const &input,
          std::shared_ptr<scope> const &outer_scope
        )
        {
          static std::size_t constexpr forms_required{ 4 };

          auto &data(input.data);
          if(data.size() < forms_required)
          {
            throw expect::error::syntax::exception<>
            { "invalid macro definition" };
          }

          auto const name
          (parse::expect::type<parse::cell::type::ident>(data[1]));
          auto const types
          (parse::expect::type<parse::cell::type::list>(data[2]));
          auto const args
          (parse::expect::type<parse::cell::type::list>(data[3]));
          auto const nested_scope
          (std::make_shared<scope>(outer_scope));
          auto const arg_definitions
          (
            function::argument::definition::parse_types<cell::cell>
            (args, nested_scope)
          );

          if(types.data.size())
          {
            throw expect::error::internal::unimplemented
            { "generic macros" };
          }

          /* Add args to macro's scope. */
          std::transform
          (
            arg_definitions.begin(), arg_definitions.end(),
            std::inserter
            (
              nested_scope->binding_definitions,
              nested_scope->binding_definitions.end()
            ),
            [](auto const &arg)
            {
              return std::make_pair
              (
                arg.name,
                cell::binding_definition
                { {
                  arg.name, arg.type, {}
                } }
              );
            }
          );

          for
          (
            auto const &overload :
            outer_scope->macro_definitions[name.data]
          )
          {
            if(overload.data.arguments == arg_definitions)
            {
              throw expect::error::type::overload
              { "multiple definition of: " + name.data };
            }
          }

          /* Add an empty declaration first, to allow for recursive references. */
          auto &decls(outer_scope->macro_definitions[name.data]);
          decls.emplace_back();
          auto &decl(decls.back());
          decl.data.name = name.data;
          decl.data.arguments = arg_definitions;

          cell::macro_definition ret
          {
            {
              name.data,
              arg_definitions,
              translate /* Recurse into translate for the body. */
              (
                jtl::it::make_range
                (
                  std::next
                  (
                    data.begin(),
                    forms_required
                  ),
                  data.end()
                ),
                nested_scope,
                { environment::builtin::type::automatic(*outer_scope) }
              ).data,
              nested_scope
            }
          };

          /* Add the macro definition to the outer body's scope. */
          outer_scope->macro_definitions[name.data].back() = ret;
          return { ret };
        }
      }
    }
  }
}
