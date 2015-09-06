#include <jank/translate/environment/special/binding.hpp>

#include <jank/parse/expect/type.hpp>
#include <jank/translate/translate.hpp>
#include <jank/translate/type/generic/extract.hpp>
#include <jank/translate/type/generic/parse.hpp>
#include <jank/translate/type/generic/verify.hpp>
#include <jank/translate/function/argument/resolve_type.hpp>
#include <jank/translate/environment/scope.hpp>
#include <jank/translate/environment/builtin/type/normalize.hpp>
#include <jank/translate/expect/error/syntax/exception.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    {
      namespace special
      {
        cell::cell binding
        (
          parse::cell::list const &input,
          std::shared_ptr<scope> const &outer_scope
        )
        {
          static std::size_t constexpr forms_required{ 3 };

          auto &data(input.data);
          if(data.size() < forms_required)
          {
            throw expect::error::syntax::exception<>
            { "invalid binding definition" };
          }

          auto const name
          (parse::expect::type<parse::cell::type::ident>(data[1]));

          auto const var_opt(outer_scope->find_binding(name.data));
          if(var_opt && var_opt.value().second == outer_scope)
          {
            throw expect::error::type::exception<>
            { "multiple definition of: " + name.data };
          }

          bool const deduce_type{ data.size() == 3 };

          auto it(std::next(data.begin(), 1));
          cell::type_definition expected_type;
          if(!deduce_type)
          {
            it = std::next(it);
            auto const type_name
            (parse::expect::type<parse::cell::type::ident>(*it));
            auto const type_opt
            (outer_scope->find_type(type_name.data));

            if(!type_opt)
            {
              throw expect::error::type::exception<>
              { "unknown type in binding definition" };
            }
            expected_type.data = environment::builtin::type::normalize
            (type_opt.value().first.data, *outer_scope);

            /* Parse any generics along with this type. */
            std::tie(expected_type.data, it) = type::generic::apply_genericity
            (std::move(expected_type.data), it, data.end(), outer_scope);
          }

          /* Remove everything but the type and the value and parse it as a
           * function call. The type will be skipped, as it's considered the
           * function's name. */
          auto parsable_list(input);
          parsable_list.data.erase
          (
            parsable_list.data.begin(),
            std::next
            (parsable_list.data.begin(), std::distance(data.begin(), it))
          );
          auto const arguments
          (
            function::argument::call::parse<cell::cell>
            (parsable_list, outer_scope)
          );
          if(arguments.empty())
          {
            throw expect::error::syntax::exception<>
            { "no value specified in binding definition" };
          }
          else if(arguments.size() > 1)
          {
            throw expect::error::syntax::exception<>
            { "multiple values specified in binding definition" };
          }

          auto const value_type
          (
            function::argument::resolve_type
            (arguments[0].cell, outer_scope)
          );
          if(deduce_type)
          { expected_type = value_type; }
          else
          {
            if(value_type.data != expected_type.data)
            {
              throw expect::error::type::exception<>
              { "incompatible types for binding definition" };
            }
          }

          cell::binding_definition const def
          { { name.data, { value_type.data }, arguments[0].cell } };
          outer_scope->binding_definitions[name.data] = def;
          return { def };
        }
      }
    }
  }
}
