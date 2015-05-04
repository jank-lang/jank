#include <jank/translate/cell/stream.hpp>

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      std::ostream& operator <<(std::ostream &os, cell const &c)
      {
        static int indent_level{ -1 };

        switch(static_cast<type>(c.which()))
        {
          case type::function_body:
          {
            ++indent_level;
            os << "\n";
            for(auto const i : jtl::it::make_range(0, indent_level))
            { static_cast<void>(i); os << "  "; }

            os << "( ";
            for(auto const &v : boost::get<function_body>(c).data.cells)
            { os << v << " "; }
            os << ") ";

            --indent_level;
          } break;
          case type::function_definition:
          {
            ++indent_level;
            os << "\n";
            for(auto const i : jtl::it::make_range(0, indent_level))
            { static_cast<void>(i); os << "  "; }

            auto const &def(boost::get<function_definition>(c));
            os << "function " << def.data.name << " : " << def.data.arguments << std::endl;
            os << "( ";
            for(auto const &v : def.data.body.cells)
            { os << v << " "; }
            os << ") ";

            --indent_level;
          } break;
          case type::function_call:
          {
            ++indent_level;
            os << "\n";
            for(auto const i : jtl::it::make_range(0, indent_level))
            { static_cast<void>(i); os << "  "; }

            auto const &def(boost::get<function_call>(c));
            os << "call " << def.data.name << " : " << def.data.arguments << std::endl;

            --indent_level;
          } break;
          case type::variable_definition:
          {
            ++indent_level;
            os << "\n";
            for(auto const i : jtl::it::make_range(0, indent_level))
            { static_cast<void>(i); os << "  "; }

            auto const &def(boost::get<variable_definition>(c));
            os << "var " << def.data.name << std::endl;

            --indent_level;
          } break;
          default:
            os << "??? ";
        }

        return os;
      }
    }
  }
}
