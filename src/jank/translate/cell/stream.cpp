#include <jank/parse/cell/stream.hpp>
#include <jank/translate/cell/stream.hpp>
#include <jank/translate/cell/trait.hpp>
#include <jank/translate/function/argument/call.hpp>
#include <jank/detail/stream/indenter.hpp>

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      static std::ostream& operator <<
      (std::ostream &os, type_definition::type const &c)
      {
        os << "( " << trait::to_string<type::type_definition>() << " "
           << "( name " << c.name << " ) "
           << "( members ";

        /* TODO: members */

        os << ") "
           << "( generics ";

        /* TODO: generics */

        os << ") ";
        return os << ") ";
      }

      static std::ostream& operator <<
      (std::ostream &os, type_reference::type const &c)
      {
        os << "( " << trait::to_string<type::type_reference>() << " "
           << "( definition " << c.definition << ") ) ";
        return os;
      }

      static std::ostream& operator <<
      (std::ostream &os, binding_definition::type const &c)
      {
        os << "( " << trait::to_string<type::binding_definition>() << " "
           << "( name " << c.name << " ) "
           << "( type " << c.type << ") "
           << "( cell " << c.cell << ") ) ";
        return os;
      }

      static std::ostream& operator <<
      (std::ostream &os, binding_reference::type const &c)
      {
        os << "( " << trait::to_string<type::binding_reference>() << " "
           << "( definition " << c.definition << ") ) ";
        return os;
      }

      static std::ostream& operator <<
      (std::ostream &os, literal_value::type const &c)
      {
        os << "( " << trait::to_string<type::literal_value>() << " "
           << "( type ";

        switch(static_cast<literal_type>(c.which()))
        {
          case literal_type::null:
            os << parse::cell::trait::to_string<parse::cell::type::null>()
               << " ) "
               << "( value null ) ";
            break;
          case literal_type::boolean:
            os << parse::cell::trait::to_string<parse::cell::type::boolean>()
               << " ) "
               << "( value " << boost::get<parse::cell::boolean>(c).data
               << " ) ";
            break;
          case literal_type::integer:
            os << parse::cell::trait::to_string<parse::cell::type::integer>()
               << " ) "
               << "( value " << boost::get<parse::cell::integer>(c).data
               << " ) ";
            break;
          case literal_type::real:
            os << parse::cell::trait::to_string<parse::cell::type::real>()
               << " ) "
               << "( value " << boost::get<parse::cell::real>(c).data
               << " ) ";
            break;
          case literal_type::string:
            os << parse::cell::trait::to_string<parse::cell::type::string>()
               << " ) "
               << "( value \"" << boost::get<parse::cell::string>(c).data
               << "\" ) ";
            break;
          case literal_type::list:
            /* TODO */
            os << "list ) ( value ) ";
        }

        return os << ") ";
      }

      static std::ostream& operator <<
      (std::ostream &os, function_body::type const &c)
      {
        os << "( " << trait::to_string<type::function_body>() << " "
           << "( return-type " << c.return_type << " ) "
           << "( cells ";
        for(auto const &cell : c.cells)
        { os << cell; }
        return os << ") ) ";
      }

      static std::ostream& operator <<
      (std::ostream &os, function_definition::type const &c)
      {
        os << "( " << trait::to_string<type::function_definition>() << " "
           << "( name " << c.name << " ) "
           << "( arguments ";

        for(auto const &arg : c.arguments)
        {
          os << "( argument "
             << "( name " << arg.name << " ) "
             << "( type " << arg.type << ") ) ";
        }

        os << ") "
           << "( return-type " << c.return_type << ") "
           << "( body " << c.body << ") ) ";
        return os;
      }

      static std::ostream& operator <<
      (std::ostream &os, native_function_declaration::type const &c)
      {
        os << "( " << trait::to_string<type::native_function_declaration>() << " "
           << "( name " << c.name << " ) "
           << "( arguments ";

        for(auto const &arg : c.arguments)
        {
          os << "( argument "
             << "( name " << arg.name << " ) "
             << "( type " << arg.type << ") ) ";
        }

        os << ") "
           << "( return-type " << c.return_type << ") ) ";
        return os;
      }

      static std::ostream& operator <<
      (std::ostream &os, function_call::type const &c)
      {
        os << "( " << trait::to_string<type::function_call>() << " "
           << "( name " << c.definition.name << " ) "
           << "( arguments ";

        for(auto const &arg : c.arguments)
        {
          os << "( argument "
             << "( name " << arg.name << " ) "
             << "( cell " << arg.cell << ") ) ";
        }

        return os << ") ) ";
      }

      static std::ostream& operator <<
      (std::ostream &os, indirect_function_call::type const &c)
      {
        os << "( " << trait::to_string<type::indirect_function_call>() << " "
           << "( binding " << c.binding << " ) "
           << "( arguments ";

        for(auto const &arg : c.arguments)
        {
          os << "( argument "
             << "( name " << arg.name << " ) "
             << "( cell " << arg.cell << ") ) ";
        }

        return os << ") ) ";
      }

      static std::ostream& operator <<
      (std::ostream &os, native_function_call::type const &c)
      {
        os << "( " << trait::to_string<type::native_function_call>() << " "
           << "( name " << c.definition.name << " ) "
           << "( arguments ";

        for(auto const &arg : c.arguments)
        {
          os << "( argument "
             << "( name " << arg.name << " ) "
             << "( cell " << arg.cell << ") ) ";
        }

        return os << ") ) ";
      }

      static std::ostream& operator <<
      (std::ostream &os, function_reference::type const &c)
      {
        os << "( " << trait::to_string<type::function_reference>() << " "
           << "( name " << c.definition.name << " ) ";
        return os << ") ";
      }

      static std::ostream& operator <<
      (std::ostream &os, native_function_reference::type const &c)
      {
        os << "( " << trait::to_string<type::native_function_reference>() << " "
           << "( name " << c.definition.name << " ) ";
        return os << ") ";
      }

      static std::ostream& operator <<
      (std::ostream &os, return_statement::type const &c)
      {
        os << "( " << trait::to_string<type::return_statement>() << " "
           << "( cell " << c.cell << ") "
           << "( expected-type " << c.expected_type << ") ) ";
        return os;
      }

      static std::ostream& operator <<
      (std::ostream &os, if_statement::type const &c)
      {
        os << "( " << trait::to_string<type::if_statement>() << " "
           << "( condition " << c.condition << ") "
           << "( true-body " << c.true_body << ") "
           << "( false-body " << c.false_body << ") ) ";
        return os;
      }

      static std::ostream& operator <<
      (std::ostream &os, do_statement::type const &c)
      {
        os << "( " << trait::to_string<type::do_statement>() << " "
           << "( body " << c.body << ") ) ";
        return os;
      }

      std::ostream& operator <<(std::ostream &os, cell const &c)
      {
        switch(trait::to_enum(c))
        {
          case type::type_definition:
            os << boost::get<type_definition>(c).data;
            break;
          case type::type_reference:
            os << boost::get<type_reference>(c).data;
            break;
          case type::function_body:
            os << boost::get<function_body>(c).data;
            break;
          case type::function_definition:
            os << boost::get<function_definition>(c).data;
            break;
          case type::native_function_declaration:
            os << boost::get<native_function_declaration>(c).data;
            break;
          case type::function_call:
            os << boost::get<function_call>(c).data;
            break;
          case type::indirect_function_call:
            os << boost::get<indirect_function_call>(c).data;
            break;
          case type::native_function_call:
            os << boost::get<native_function_call>(c).data;
            break;
          case type::function_reference:
            os << boost::get<function_reference>(c).data;
            break;
          case type::native_function_reference:
            os << boost::get<native_function_reference>(c).data;
            break;
          case type::binding_definition:
            os << boost::get<binding_definition>(c).data;
            break;
          case type::binding_reference:
            os << boost::get<binding_reference>(c).data;
            break;
          case type::literal_value:
            os << boost::get<literal_value>(c).data;
            break;
          case type::return_statement:
            os << boost::get<return_statement>(c).data;
            break;
          case type::if_statement:
            os << boost::get<if_statement>(c).data;
            break;
          case type::do_statement:
            os << boost::get<do_statement>(c).data;
            break;
          default:
            os << "??? ";
            break;
        }

        return os;
      }
    }
  }
}
