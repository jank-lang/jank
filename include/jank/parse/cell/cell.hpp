#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <cstdint>

#include <boost/variant.hpp>

#include <jtl/iterator/range.hpp>

#include <jank/parse/cell/type.hpp>

namespace jank
{
  namespace parse
  {
    namespace cell
    {
      template <type C>
      struct wrapper;

      using cell = boost::variant
      <
        boost::recursive_wrapper<wrapper<type::boolean>>,
        boost::recursive_wrapper<wrapper<type::integer>>,
        boost::recursive_wrapper<wrapper<type::real>>,
        boost::recursive_wrapper<wrapper<type::string>>,
        boost::recursive_wrapper<wrapper<type::ident>>,
        boost::recursive_wrapper<wrapper<type::list>>,
        boost::recursive_wrapper<wrapper<type::function>>
      >;

      template <>
      struct wrapper<type::boolean>
      {
        using type = bool;
        type data;
      };
      template <>
      struct wrapper<type::integer>
      {
        using type = int64_t;
        type data;
      };
      template <>
      struct wrapper<type::real>
      {
        using type = double;
        type data;
      };
      template <>
      struct wrapper<type::string>
      {
        using type = std::string;
        type data;
      };
      template <>
      struct wrapper<type::ident>
      {
        using type = std::string;
        type data;
      };
      template <>
      struct wrapper<type::list>
      {
        using type = std::vector<cell>;
        type data;
      };

      using boolean = wrapper<type::boolean>;
      using integer = wrapper<type::integer>;
      using real = wrapper<type::real>;
      using string = wrapper<type::string>;
      using ident = wrapper<type::ident>;
      using list = wrapper<type::list>;
      using func = wrapper<type::function>;

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

      namespace trait
      {
        template <type C>
        struct type_variant;
        template <>
        struct type_variant<type::boolean>
        { using type = boolean; };
        template <>
        struct type_variant<type::integer>
        { using type = integer; };
        template <>
        struct type_variant<type::real>
        { using type = real; };
        template <>
        struct type_variant<type::string>
        { using type = string; };
        template <>
        struct type_variant<type::ident>
        { using type = ident; };
        template <>
        struct type_variant<type::list>
        { using type = list; };
        template <>
        struct type_variant<type::function>
        { using type = func; };
      }
      template <type C>
      using type_variant = typename trait::type_variant<C>::type;
    }
  }
}
