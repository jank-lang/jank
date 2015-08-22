#include <algorithm>
#include <memory>

#include <jank/translate/function/argument/call.hpp>
#include <jank/parse/cell/cell.hpp>
#include <jank/parse/cell/trait.hpp>
#include <jank/parse/expect/type.hpp>
#include <jank/translate/environment/scope.hpp>
#include <jank/translate/environment/special/apply_expression.hpp>
#include <jank/translate/environment/special/apply_definition.hpp>
#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/function/match_overload.hpp>
#include <jank/translate/function/return/deduce.hpp>
#include <jank/translate/function/return/validate.hpp>
#include <jank/translate/expect/error/type/exception.hpp>
#include <jank/translate/expect/error/syntax/exception.hpp>
#include <jank/translate/expect/error/lookup/exception.hpp>
#include <jank/translate/expect/error/internal/unimplemented.hpp>

namespace jank
{
  namespace translate
  {
    namespace function
    {
      namespace argument
      {
        namespace call
        {
          template <typename C>
          class visitor
          {
            public:
              visitor() = delete;
              visitor(std::shared_ptr<environment::scope> const &s)
                : scope_{ s }
              { }

              template <typename T>
              detail::value<C> operator ()(T const&) const
              {
                throw expect::error::type::exception<>
                {
                  std::string{ "invalid argument type: " } +
                  parse::cell::trait::to_string
                  <parse::cell::trait::to_enum<T>()>()
                };
              }

              detail::value<C> operator ()(parse::cell::null const &c) const
              { return call(c); }
              detail::value<C> operator ()(parse::cell::boolean const &c) const
              { return call(c); }
              detail::value<C> operator ()(parse::cell::integer const &c) const
              { return call(c); }
              detail::value<C> operator ()(parse::cell::real const &c) const
              { return call(c); }
              detail::value<C> operator ()(parse::cell::string const &c) const
              { return call(c); }
              detail::value<C> operator ()(parse::cell::ident const &c) const
              {
                auto const binding(scope_->find_binding(c.data));
                if(binding)
                {
                  return detail::value<C>
                  {
                    c.data,
                    { cell::binding_reference{ { binding.value().first.data } } }
                  };
                }

                auto const function(scope_->find_function(c.data));
                if(function)
                {
                  auto const &overloads(function.value());
                  if(overloads.size() != 1)
                  {
                    throw expect::error::type::overload
                    { "ambiguous function reference" };
                  }

                  return detail::value<C>
                  {
                    c.data,
                    { cell::function_reference{ { overloads[0].first.data } } }
                  };
                }

                auto const native_function(scope_->find_native_function(c.data));
                if(native_function)
                {
                  auto const &overloads(native_function.value());
                  if(overloads.size() != 1)
                  {
                    throw expect::error::type::overload
                    { "ambiguous native function reference" };
                  }

                  return detail::value<C>
                  {
                    c.data,
                    { cell::native_function_reference{ { overloads[0].first.data } } }
                  };
                }

                throw expect::error::type::exception<>
                { "unknown binding: " + c.data };
              }
              detail::value<C> operator ()(parse::cell::list const &c) const
              {
                if(c.data.empty())
                {
                  throw expect::error::syntax::exception<>
                  { "invalid argument list" };
                }

                /* See if it's a special definition. */
                {
                  cell::function_body body
                  { {
                    {},
                    environment::builtin::type::automatic(*scope_),
                    scope_
                  } };

                  auto const special_opt
                  (environment::special::apply_definition(c, body));
                  if(special_opt)
                  {
                    return detail::value<C>
                    {
                      "special", /* TODO: Better name? */
                      { special_opt.value() }
                    };
                  }
                }
                /* See if it's a special expression. */
                {
                  cell::function_body body
                  { {
                    {},
                    environment::builtin::type::automatic(*scope_),
                    scope_
                  } };

                  auto const special_opt
                  (environment::special::apply_expression(c, body));
                  if(special_opt)
                  {
                    body.data.cells.push_back(special_opt.value());
                    body.data = function::ret::deduce
                    (function::ret::validate(std::move(body.data)));

                    return detail::value<C>
                    {
                      "special", /* TODO: Better name? */
                      { body }
                    };
                  }
                }

                auto const name
                (parse::expect::type<parse::cell::type::ident>(c.data[0]).data);
                auto const native_function_opt
                (scope_->find_native_function(name));
                auto const function_opt(scope_->find_function(name));

                /* TODO: This mutation over a closure is shitty. */
                detail::value<C> ret;
                function::match_overload
                (
                  c, scope_, native_function_opt, function_opt,
                  [&](auto const &match)
                  {
                    ret = detail::value<C>
                    {
                      name,
                      { match }
                    };
                  }
                );
                return ret;
              }

            private:
              template <typename T>
              detail::value<C> call(T const &c) const
              {
                return detail::value<C>
                {
                  std::string{ "rvalue " } +
                  parse::cell::trait::to_string
                  <
                    parse::cell::trait::to_enum<T>()
                  >(),
                  { cell::literal_value{ c } }
                };
              }

              std::shared_ptr<environment::scope> scope_;
          };

          /* TODO: Have this not skip the first item.
           * Instead, it should only be passed the args. */
          template <>
          value_list<cell::cell> parse<cell::cell>
          (
            parse::cell::list const &l,
            std::shared_ptr<environment::scope> const &scope
          )
          {
            value_list<cell::cell> ret;

            /* No parameters to parse. */
            if(l.data.empty())
            { return ret; }

            std::transform
            (
              std::next(l.data.begin(), 1), l.data.end(),
              std::back_inserter(ret),
              [&](auto const &a) -> detail::value<cell::cell>
              { return parse::cell::visit(a, visitor<cell::cell>{ scope }); }
            );

            return ret;
          }
        }
      }
    }
  }
}
