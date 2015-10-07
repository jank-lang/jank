#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/environment/builtin/type/list.hpp>
#include <jank/translate/plugin/detail/make_function.hpp>

namespace jank
{
  namespace translate
  {
    namespace plugin
    {
      namespace io
      {
        void print(std::shared_ptr<environment::scope> const &scope)
        {
          detail::make_function
          (
            scope, "print",
            environment::builtin::type::null(*scope),
            environment::builtin::type::null(*scope)
          );
          detail::make_function
          (
            scope, "print",
            environment::builtin::type::null(*scope),
            environment::builtin::type::boolean(*scope)
          );
          detail::make_function
          (
            scope, "print",
            environment::builtin::type::null(*scope),
            environment::builtin::type::integer(*scope)
          );
          detail::make_function
          (
            scope, "print",
            environment::builtin::type::null(*scope),
            environment::builtin::type::real(*scope)
          );
          detail::make_function
          (
            scope, "print",
            environment::builtin::type::null(*scope),
            environment::builtin::type::string(*scope)
          );
          detail::make_function
          (
            scope, "print",
            environment::builtin::type::null(*scope),
            environment::builtin::type::list
            (
              *scope,
              environment::builtin::type::integer(*scope)
            )
          );
        }
      }
    }
  }
}
