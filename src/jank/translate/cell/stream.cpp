#include <jank/translate/cell/stream.hpp>
#include <jank/parse/cell/stream.hpp>

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      struct indenter
      {
        indenter(std::ostream &os)
        {
          ++level;
          os << "\n";
          for(auto const i : jtl::it::make_range(0, level))
          { static_cast<void>(i); os << "  "; }
        }
        ~indenter()
        { --level; }

        static int level;
      };
      int indenter::level{ -1 };

      std::ostream& operator <<(std::ostream &os, function_body const &c)
      {
        indenter const indent{ os };

        os << "( ";
        for(auto const &v : c.data.cells)
        { os << v << " "; }
        os << ") ";

        return os;
      }

      std::ostream& operator <<(std::ostream &os, function_definition const &c)
      {
        indenter const indent{ os };

        os << "function " << c.data.name << " : " << c.data.arguments << std::endl;
        os << "( ";
        for(auto const &v : c.data.body.cells)
        { os << v << " "; }
        os << ") ";

        return os;
      }

      std::ostream& operator <<(std::ostream &os, function_call const &c)
      {
        indenter const indent{ os };
        return os << "call "
                  << c.data.name << " : "
                  << c.data.arguments << std::endl;
      }

      std::ostream& operator <<(std::ostream &os, variable_definition const &c)
      {
        indenter const indent{ os };
        return os << "variable " << c.data.name << std::endl;
      }

      std::ostream& operator <<(std::ostream &os, variable_reference const &c)
      {
        indenter const indent{ os };
        return os << "variable reference " << c.data.definition.name << std::endl;
      }

      std::ostream& operator <<(std::ostream &os, literal_value const &c)
      {
        indenter const indent{ os };
        return os << "value " << c.data << std::endl;
      }

      std::ostream& operator <<(std::ostream &os, cell const &c)
      {
        switch(static_cast<type>(c.which()))
        {
          case type::function_body:
            os << boost::get<function_body>(c);
            break;
          case type::function_definition:
            os << boost::get<function_definition>(c);
            break;
          case type::function_call:
            os << boost::get<function_call>(c);
            break;
          case type::variable_definition:
            os << boost::get<variable_definition>(c);
            break;
          case type::variable_reference:
            os << boost::get<variable_reference>(c);
            break;
          case type::literal_value:
            os << boost::get<literal_value>(c);
            break;
          default:
            os << "??? ";
        }

        return os;
      }
    }
  }
}
