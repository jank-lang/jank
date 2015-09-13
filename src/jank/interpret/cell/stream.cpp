#include <jank/interpret/cell/stream.hpp>
#include <jank/interpret/cell/trait.hpp>
#include <jank/detail/stream/indenter.hpp>

namespace jank
{
  namespace interpret
  {
    namespace cell
    {
      static int indent_level{ -1 };

      static std::ostream& operator <<(std::ostream &os, null const &)
      { return os << "null"; }

      static std::ostream& operator <<(std::ostream &os, boolean const &c)
      { return os << std::boolalpha << c.data; }

      static std::ostream& operator <<(std::ostream &os, integer const &c)
      { return os << c.data; }

      static std::ostream& operator <<(std::ostream &os, real const &c)
      { return os << c.data; }

      static std::ostream& operator <<(std::ostream &os, string const &c)
      { return os << c.data; }

      static std::ostream& operator <<(std::ostream &os, function const &)
      {
        detail::stream::indenter const indent{ os, indent_level };
        return os << "function";
      }

      static std::ostream& operator <<(std::ostream &os, list const &l)
      {
        detail::stream::indenter const indent{ os, indent_level };
        os << "(";
        for(auto const &e : l.data)
        { os << e << " "; }
        return os << ")";
      }

      std::ostream& operator <<(std::ostream &os, cell const &c)
      {
        switch(trait::to_enum(c))
        {
          case type::null:
            os << boost::get<null>(c);
            break;
          case type::boolean:
            os << boost::get<boolean>(c);
            break;
          case type::integer:
            os << boost::get<integer>(c);
            break;
          case type::real:
            os << boost::get<real>(c);
            break;
          case type::string:
            os << boost::get<string>(c);
            break;
          case type::function:
            os << boost::get<function>(c);
            break;
          case type::list:
            os << boost::get<list>(c);
            break;
          default:
            os << "??? ";
        }

        return os;
      }
    }
  }
}
