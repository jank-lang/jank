#include <jank/translate/environment/special/variable.hpp>

#include <jank/parse/expect/type.hpp>
#include <jank/translate/translate.hpp>
#include <jank/translate/environment/scope.hpp>
#include <jank/translate/expect/error/syntax/syntax.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    {
      namespace special
      {
        cell::cell variable
        (parse::cell::list const &input, cell::function_body const &)
        {
          static std::size_t constexpr forms_required{ 4 };

          auto &data(input.data);
          if(data.size() < forms_required)
          { throw expect::error::syntax::exception<>{ "invalid variable definition" }; }

          auto const name(parse::expect::type<parse::cell::type::ident>(data[1]));
          auto const type(parse::expect::type<parse::cell::type::ident>(data[2]));
          return {};
        }
      }
    }
  }
}
