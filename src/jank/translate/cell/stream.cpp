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
      (std::ostream &os, function_body::type const &c)
      {
        os << "( " << trait::to_string<type::function_body>() << " "
           << "( return-type " << /*c.return_type <<*/ " ) "
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
        { os << "( " << arg.name << " " /*<< arg.type*/ << ") "; }

        os << ") "
           << "( return-type " << /*c.return_type <<*/ ") "
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
        { os << "( " << arg.name << " " /*<< arg.type*/ << ") "; }

        os << ") "
           << "( return-type " << /*c.return_type <<*/ ") ) ";
        return os;
      }

      static std::ostream& operator <<
      (std::ostream &os, function_call::type const &c)
      {
        os << "( " << trait::to_string<type::function_call>() << " "
           << "( name " << c.definition.name << " ) "
           << "( arguments ";

        for(auto const &arg : c.arguments)
        { os << "( " << arg.name << " " /*<< arg.value*/ << ") "; }

        return os << ") ) ";
      }

      static std::ostream& operator <<
      (std::ostream &os, indirect_function_call::type const &c)
      {
        os << "( " << trait::to_string<type::indirect_function_call>() << " "
           << "( binding " << /*c.binding <<*/ " ) "
           << "( arguments ";

        for(auto const &arg : c.arguments)
        { os << "( " << arg.name << " " /*<< arg.value*/ << ") "; }

        return os << ") ) ";
      }

      static std::ostream& operator <<
      (std::ostream &os, native_function_call::type const &c)
      {
        os << "( " << trait::to_string<type::native_function_call>() << " "
           << "( name " << c.definition.name << " ) "
           << "( arguments ";

        for(auto const &arg : c.arguments)
        { os << "( " << arg.name << " " /*<< arg.value*/ << ") "; }

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
      (std::ostream &os, binding_definition::type const &c)
      {
        os << "( " << trait::to_string<type::binding_definition>() << " "
           << "( name " << c.name << " ) "
           //<< c.type << ") "
           << "( cell " << c.cell << ") ";
        return os;
      }

      std::ostream& operator <<(std::ostream &os, cell const &c)
      {
        switch(trait::to_enum(c))
        {
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
            //os << boost::get<binding_reference>(c).data;
            break;
          case type::literal_value:
            //os << boost::get<literal_value>(c).data;
            break;
          case type::return_statement:
            //os << boost::get<return_statement>(c).data;
            break;
          case type::if_statement:
            //os << boost::get<if_statement>(c).data;
            break;
          case type::do_statement:
            //os << boost::get<do_statement>(c).data;
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
