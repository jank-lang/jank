#include <algorithm>
#include <memory>

#include <jank/translate/function/argument/call.hpp>
#include <jank/parse/cell/cell.hpp>
#include <jank/parse/cell/trait.hpp>
#include <jank/parse/expect/type.hpp>
#include <jank/interpret/environment/scope.hpp> /* TODO: for function */
#include <jank/translate/environment/scope.hpp>
#include <jank/translate/function/match_overload.hpp>
#include <jank/translate/expect/error/type/type.hpp>
#include <jank/translate/expect/error/syntax/syntax.hpp>
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
              detail::argument_value<C> operator ()(T const&) const
              {
                throw expect::error::type::exception<>
                {
                  std::string{ "invalid argument type: " } +
                  parse::cell::trait::to_string
                  <parse::cell::trait::to_enum<T>()>()
                };
              }

              detail::argument_value<C> operator ()(parse::cell::null const &c) const
              { return call(c); }
              detail::argument_value<C> operator ()(parse::cell::boolean const &c) const
              { return call(c); }
              detail::argument_value<C> operator ()(parse::cell::integer const &c) const
              { return call(c); }
              detail::argument_value<C> operator ()(parse::cell::real const &c) const
              { return call(c); }
              detail::argument_value<C> operator ()(parse::cell::string const &c) const
              { return call(c); }
              detail::argument_value<C> operator ()(parse::cell::ident const &c) const
              {
                auto const def(scope_->find_variable(c.data));
                if(!def)
                { throw expect::error::type::exception<>{ "unknown variable: " + c.data }; }

                return detail::argument_value<C>
                {
                  c.data,
                  { cell::variable_reference{ def.value().data } }
                };
              }
              detail::argument_value<C> operator ()(parse::cell::list const &c) const
              {
                if(c.data.empty())
                { throw expect::error::syntax::exception<>{ "invalid argument list" }; }

                auto const name(parse::expect::type<parse::cell::type::ident>(c.data[0]).data);
                auto const function_opt(scope_->find_function(name));
                if(function_opt)
                {
                  auto const matched_opt(function::match_overload(c, scope_, function_opt.value()));
                  if(matched_opt)
                  {
                    return detail::argument_value<C>
                    {
                      name,
                      { matched_opt.value() }
                    };
                  }
                  else
                  {
                    throw expect::error::type::exception<>
                    { "invalid function call: " + name };
                  }
                }
                else
                {
                  throw expect::error::type::exception<>
                  { "unknown function: " + name };
                }
              }

            private:
              template <typename T>
              detail::argument_value<C> call(T const &c) const
              {
                return detail::argument_value<C>
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
              [&](auto const &a) -> detail::argument_value<cell::cell>
              { return parse::cell::visit(a, visitor<cell::cell>{ scope }); }
            );

            return ret;
          }
        }
      }
    }
  }
}
