#include <jank/translate/function/match_indirect.hpp>

namespace jank
{
  namespace translate
  {
    namespace function
    {
      void match_indirect
      (
        cell::type_definition::type const &type,
        parse::cell::list const &args,
        std::shared_ptr<environment::scope> const &scope,
        std::function<void (cell::indirect_function_call)> callback
      )
      {
        /* TODO: Parse args, match args against type, throw on failure. */
        static_cast<void>(type);
        static_cast<void>(args);
        static_cast<void>(scope);
        static_cast<void>(callback);
      }
    }
  }
}
