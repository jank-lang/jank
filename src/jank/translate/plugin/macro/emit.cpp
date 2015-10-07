#include <jank/translate/plugin/macro/emit.hpp>
#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/environment/builtin/type/macro_primitive.hpp>
#include <jank/translate/environment/builtin/value/primitive.hpp>
#include <jank/translate/plugin/detail/make_function.hpp>

namespace jank
{
  namespace translate
  {
    namespace plugin
    {
      namespace macro
      {
        void emit(std::shared_ptr<environment::scope> const &scope)
        {
          detail::make_function
          (
            scope, "emit",
            environment::builtin::type::null(*scope),
            environment::builtin::type::macro_atom(*scope)
          );
          detail::make_function
          (
            scope, "emit",
            environment::builtin::type::null(*scope),
            environment::builtin::type::macro_list(*scope)
          );
        }
      }
    }
  }
}
