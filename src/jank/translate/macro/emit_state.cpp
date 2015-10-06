#include <jank/translate/macro/emit_state.hpp>
#include <jank/translate/expect/error/internal/exception.hpp>

namespace jank
{
  namespace translate
  {
    namespace macro
    {
      namespace detail
      { static std::vector<std::reference_wrapper<emit_state>> stack; }

      emit_state::emit_state()
      { detail::stack.push_back(std::ref(*this)); }

      emit_state::~emit_state()
      { detail::stack.pop_back(); }

      void emit(parse::cell::cell const &cell)
      {
        if(detail::stack.empty())
        {
          throw expect::error::internal::exception<>
          { "invalid emit stack" };
        }
        detail::stack.back().get().cells.push_back(cell);
      }
    }
  }
}
