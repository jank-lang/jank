#include <jank/parse/cell/stream.hpp>
#include <jank/translate/cell/stream.hpp>
#include <jank/translate/cell/trait.hpp>
#include <jank/translate/function/argument/call.hpp>
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

        os << "( : " << c.data.return_type.definition.name << " ";
        for(auto const &v : c.data.cells)
        { os << v << " "; }
        os << ") ";

        return os;
      }

      static std::ostream& operator <<(std::ostream &os, function_definition const &c)
      {
        indenter const indent{ os, indent_level };

        os << "function " << c.data.name
           << " : " << c.data.arguments
           << " : " << c.data.return_type.definition.name
           << std::endl;
        os << "( ";
        for(auto const &v : c.data.body.cells)
        { os << v << " "; }
        os << ") ";

        return os;
      }

      static std::ostream& operator <<(std::ostream &os, native_function_definition const &c)
      {
        indenter const indent{ os, indent_level };

        os << "native function " << c.data.name
           << " : " << c.data.arguments
           << " : " << c.data.return_type.definition.name
           << std::endl;

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

      static std::ostream& operator <<(std::ostream &os, indirect_function_call const &c)
      {
        indenter const indent{ os, indent_level };
        os << "indirect call "
           << c.data.binding.name << " : ";
        for(auto const &a : c.data.arguments)
        {
          os << "( " << a.name << " "
             << trait::to_string(trait::to_enum(a.cell))
             << " ) ";
        }
        return os << std::endl;
      }

      static std::ostream& operator <<(std::ostream &os, native_function_call const &c)
      {
        indenter const indent{ os, indent_level };
        os << "native call "
           << c.data.definition.name << " : ";
        for(auto const &a : c.data.arguments)
        {
          os << "( " << a.name << " "
             << trait::to_string(trait::to_enum(a.cell))
             << " ) ";
        }
        return os << std::endl;
      }

      static std::ostream& operator <<(std::ostream &os, function_reference const &c)
      {
        indenter const indent{ os, indent_level };
        /* TODO: Generic information. */
        os << "function reference "
           << c.data.definition.name;
        return os << std::endl;
      }

      static std::ostream& operator <<(std::ostream &os, native_function_reference const &c)
      {
        indenter const indent{ os, indent_level };
        /* TODO: Generic information. */
        os << "native function reference "
           << c.data.definition.name;
        return os << std::endl;
      }

      static std::ostream& operator <<(std::ostream &os, binding_definition const &c)
      {
        indenter const indent{ os, indent_level };
        return os << "binding " << c.data.name << " : "
                  << c.data.type.definition.name
                  << std::endl;
      }

      static std::ostream& operator <<(std::ostream &os, binding_reference const &c)
      {
        indenter const indent{ os, indent_level };
        return os << "binding reference " << c.data.definition.name << std::endl;
      }

      static std::ostream& operator <<(std::ostream &os, literal_value const &c)
      {
        indenter const indent{ os, indent_level };
        os << "literal ";
        switch(static_cast<literal_type>(c.data.which()))
        {
          case literal_type::null:
            break;
          case literal_type::boolean:
            os << "boolean ";
            break;
          case literal_type::integer:
            os << "integer ";
            break;
          case literal_type::real:
            os << "real ";
            break;
          case literal_type::string:
            os << "string ";
            break;
          case literal_type::ident:
            os << "ident ";
            break;
        }
        return os << c.data << std::endl;
      }

      static std::ostream& operator <<(std::ostream &os, return_statement const &c)
      {
        indenter const indent{ os, indent_level };
        return os << "return " << c.data.cell << std::endl;
      }

      static std::ostream& operator <<(std::ostream &os, if_statement const &c)
      {
        indenter const indent{ os, indent_level };
        os << "if " << c.data.condition << std::endl;
        os << function_body{ c.data.true_body } << std::endl;
        os << "else" << std::endl;
        return os << function_body{ c.data.false_body } << std::endl;
      }

      static std::ostream& operator <<(std::ostream &os, do_statement const &c)
      {
        indenter const indent{ os, indent_level };
        return os << "do " << function_body{ c.data.body } << std::endl;
      }

      std::ostream& operator <<(std::ostream &os, cell const &c)
      {
        switch(trait::to_enum(c))
        {
          case type::function_body:
            os << boost::get<function_body>(c);
            break;
          case type::function_definition:
            os << boost::get<function_definition>(c);
            break;
          case type::native_function_definition:
            os << boost::get<native_function_definition>(c);
            break;
          case type::function_call:
            os << boost::get<function_call>(c);
            break;
          case type::indirect_function_call:
            os << boost::get<indirect_function_call>(c);
            break;
          case type::native_function_call:
            os << boost::get<native_function_call>(c);
            break;
          case type::function_reference:
            os << boost::get<function_reference>(c);
            break;
          case type::native_function_reference:
            os << boost::get<native_function_reference>(c);
            break;
          case type::binding_definition:
            os << boost::get<binding_definition>(c);
            break;
          case type::binding_reference:
            os << boost::get<binding_reference>(c);
            break;
          case type::literal_value:
            os << boost::get<literal_value>(c);
            break;
          case type::return_statement:
            os << boost::get<return_statement>(c);
            break;
          case type::if_statement:
            os << boost::get<if_statement>(c);
            break;
          case type::do_statement:
            os << boost::get<do_statement>(c);
            break;
          default:
            os << "??? ";
        }

        return os;
      }
    }
  }
}
