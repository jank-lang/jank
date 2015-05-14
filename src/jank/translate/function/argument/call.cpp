#include <algorithm>
#include <memory>

#include <jank/translate/function/argument/call.hpp>
#include <jank/parse/cell/cell.hpp>
#include <jank/parse/cell/trait.hpp>
#include <jank/interpret/environment/scope.hpp> /* TODO: for function */
#include <jank/translate/environment/scope.hpp>
#include <jank/translate/expect/error/type/type.hpp>
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
          /* TODO: Read type from scope. */
          class visitor
          {
            public:
              visitor() = delete;
              visitor(std::shared_ptr<environment::scope> const &s)
                : scope{ s }
              { }

              template <typename C>
              detail::argument_value operator ()(C const&) const
              {
                throw expect::error::type::type<>
                {
                  std::string{ "invalid argument type: " } +
                  parse::cell::trait::enum_to_string
                  <parse::cell::trait::type_to_enum<C>()>()
                };
              }

              detail::argument_value operator ()(parse::cell::boolean const &c) const
              { return call(c); }
              detail::argument_value operator ()(parse::cell::integer const &c) const
              { return call(c); }
              detail::argument_value operator ()(parse::cell::real const &c) const
              { return call(c); }
              detail::argument_value operator ()(parse::cell::string const &c) const
              { return call(c); }
              detail::argument_value operator ()(parse::cell::ident const &c) const
              { return call(c); }

            private:
              template <typename C>
              detail::argument_value call(C const &c) const
              {
                return detail::argument_value
                {
                  std::string{ "rvalue " } +
                  parse::cell::trait::enum_to_string
                  <
                    parse::cell::trait::type_to_enum<C>()
                  >(),
                  { cell::literal_value{ c } }
                };
              }

              std::shared_ptr<environment::scope> scope;
          };

          value_list parse
          (
            parse::cell::list const &l,
            std::shared_ptr<environment::scope> const &scope
          )
          {
            value_list ret;

            /* No parameters to parse. */
            if(l.data.empty())
            { return ret; }

            std::transform
            (
              std::next(l.data.begin(), 1), l.data.end(),
              std::back_inserter(ret),
              [&](auto const &a) -> detail::argument_value
              { return parse::cell::visit(a, visitor{ scope }); }
            );

            return ret;
          }
        }
      }
    }
  }
}
