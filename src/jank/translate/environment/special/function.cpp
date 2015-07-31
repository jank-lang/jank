#include <stdexcept>
#include <memory>

#include <jank/parse/expect/type.hpp>
#include <jank/translate/translate.hpp>
#include <jank/translate/function/argument/definition.hpp>
#include <jank/translate/function/return/parse.hpp>
#include <jank/translate/environment/scope.hpp>
#include <jank/translate/environment/special/function.hpp>
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
        cell::cell function
        (parse::cell::list const &input, cell::function_body const &outer_body)
        {
          static std::size_t constexpr forms_required{ 3 };

          auto &data(input.data);
          if(data.size() < forms_required)
          {
            throw expect::error::syntax::exception<>
            { "invalid function definition" };
          }

          auto const name
          (parse::expect::type<parse::cell::type::ident>(data[1]));
          auto const args
          (parse::expect::type<parse::cell::type::list>(data[2]));
          auto const nested_scope
          (std::make_shared<scope>(outer_body.data.scope));
          auto const arg_definitions
          (function::argument::definition::parse_types(args, nested_scope));

          /* Add args to function's scope. */
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

          /* TODO: Check native functions, too. */
          /* Check for an already-defined function of this type. */
          /* XXX: We're only checking *this* scope's functions, so
           * shadowing is allowed. */
          for
          (
            auto const &overload :
            outer_body.data.scope->function_definitions[name.data]
          )
          {
            if(overload.data.arguments == arg_definitions)
            {
              throw expect::error::type::overload
              { "multiple definition of: " + name.data };
            }
          }

          auto return_type(builtin::type::automatic(*nested_scope));

          /* TODO: Add multiple return types into a tuple. */
          /* Parse return types. */
          /* TODO: Ambiguity between no return type and a single function
           * call and an explicit return type with an empty function body. */
          bool const return_type_provided(data.size() > 4);
          if(return_type_provided)
          {
            auto const return_type_names
            (parse::expect::type<parse::cell::type::list>(data[3]));
            auto const return_types
            (function::ret::parse(return_type_names, nested_scope));
            return_type = return_types[0].data;
          }

          /* Add an empty declaration first, to allow for recursive references. */
          auto &decls(outer_body.data.scope->function_definitions[name.data]);
          decls.emplace_back();
          auto &decl(decls.back());
          decl.data.name = name.data;
          decl.data.return_type = return_type;
          decl.data.arguments = arg_definitions;

          cell::function_definition ret
          {
            {
              name.data,
              arg_definitions,
              return_type,
              translate /* Recurse into translate for the body. */
              (
                jtl::it::make_range
                (
                  std::next
                  (
                    data.begin(),
                    forms_required + (return_type_provided ? 1 : 0)
                  ),
                  data.end()
                ),
                nested_scope,
                { return_type }
              ).data,
              nested_scope
            }
          };

          /* Verify all paths return a value. */
          ret.data.body = function::ret::validate(std::move(ret.data.body));

          /* Add the function definition to the outer body's scope. */
          outer_body.data.scope->function_definitions[name.data].back() = ret;
          return { ret };
        }
      }
    }
  }
}
