#include <jank/translate/environment/special/lambda.hpp>
#include <jank/translate/environment/special/function.hpp>
#include <jank/translate/expect/error/syntax/exception.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    {
      namespace special
      {
        cell::cell lambda
        (
          parse::cell::list const &original_input,
          std::shared_ptr<scope> const &outer_scope
        )
        {
          static std::size_t constexpr forms_required{ 3 };

          auto &data(original_input.data);
          if(data.size() < forms_required)
          {
            throw expect::error::syntax::exception<>
            { "invalid function definition" };
          }

          static std::size_t unique_id{};
          auto input(original_input);
          input.data.insert
          (
            std::next(input.data.begin()),
            parse::cell::ident{ "lambda-" + std::to_string(unique_id) }
          );
          return function(input, outer_scope);
        }
      }
    }
  }
}
