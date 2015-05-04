#include <jank/parse/cell/stream.hpp>

namespace jank
{
  namespace parse
  {
    namespace cell
    {
      std::ostream& operator <<(std::ostream &os, cell const &c)
      {
        static int indent_level{ -1 };

        switch(static_cast<type>(c.which()))
        {
          case type::boolean:
            os << std::boolalpha << boost::get<boolean>(c).data;
            break;
          case type::integer:
            os << boost::get<integer>(c).data;
            break;
          case type::real:
            os << boost::get<real>(c).data;
            break;
          case type::string:
            os << boost::get<string>(c).data;
            break;
          case type::ident:
            os << "<" << boost::get<ident>(c).data << ">";
            break;
          case type::list:
            ++indent_level;
            os << "\n";
            for(auto const i : jtl::it::make_range(0, indent_level))
            { static_cast<void>(i); os << "  "; }

            os << "( ";
            for(auto const &v : boost::get<list>(c).data)
            { os << v << " "; }
            os << ") ";

            --indent_level;
            break;
          case type::function:
            os << "function ";
            break;
          default:
            os << "??? ";
        }

        return os;
      }
    }
  }
}
