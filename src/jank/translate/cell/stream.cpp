#include <jank/translate/cell/stream.hpp>
#include <jank/translate/cell/trait.hpp>
#include <jank/translate/function/argument/call.hpp>
#include <jank/parse/cell/stream.hpp>
#include <jank/detail/stream/indenter.hpp>

namespace jank
{
  using indenter = detail::stream::indenter;
  namespace translate
  {
    namespace cell
    {
      static int indent_level{ -1 };

      static std::ostream& operator <<(std::ostream &os, function_body const &c)
      {
        indenter const indent{ os, indent_level };

        os << "( ";
        for(auto const &v : c.data.cells)
        { os << v << " "; }
        os << ") ";

        return os;
      }

      static std::ostream& operator <<(std::ostream &os, function_definition const &c)
      {
        indenter const indent{ os, indent_level };

        os << "function " << c.data.name << " : " << c.data.arguments << std::endl;
        os << "( ";
        for(auto const &v : c.data.body.cells)
        { os << v << " "; }
        os << ") ";

        return os;
      }

      static std::ostream& operator <<(std::ostream &os, function_call const &c)
      {
        indenter const indent{ os, indent_level };
        os << "call "
           << c.data.definition.name << " : ";
        for(auto const &a : c.data.arguments)
        {
          os << "( " << a.name << " "
             << trait::to_string(trait::to_enum(a.cell))
             << " ) ";
        }
        return os << std::endl;
      }

      static std::ostream& operator <<(std::ostream &os, variable_definition const &c)
      {
        indenter const indent{ os, indent_level };
        return os << "variable " << c.data.name << std::endl;
      }

      static std::ostream& operator <<(std::ostream &os, variable_reference const &c)
      {
        indenter const indent{ os, indent_level };
        return os << "variable reference " << c.data.definition.name << std::endl;
      }

      static std::ostream& operator <<(std::ostream &os, literal_value const &c)
      {
        indenter const indent{ os, indent_level };
        return os << "value " << c.data << std::endl;
      }

      static std::ostream& operator <<(std::ostream &os, return_statement const &c)
      {
        indenter const indent{ os, indent_level };
        return os << "return " << c.data.cell << std::endl;
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
          case type::return_statement:
            os << boost::get<return_statement>(c);
            break;
          default:
            os << "??? ";
        }

        return os;
      }
    }
  }
}
