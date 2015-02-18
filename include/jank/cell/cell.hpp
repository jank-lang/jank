#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <cstdint>

#include <boost/variant.hpp>

#include <jank/cell/cell_type.hpp>

namespace jank
{
  namespace cell
  {
    template <type C>
    struct wrapper;

    using cell = boost::variant
    <
      boost::recursive_wrapper<wrapper<type::integer>>,
      boost::recursive_wrapper<wrapper<type::real>>,
      boost::recursive_wrapper<wrapper<type::string>>,
      boost::recursive_wrapper<wrapper<type::ident>>,
      boost::recursive_wrapper<wrapper<type::list>>,
      boost::recursive_wrapper<wrapper<type::function>>
    >;

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

    using cell_int = wrapper<type::integer>;
    using cell_real = wrapper<type::real>;
    using cell_string = wrapper<type::string>;
    using cell_ident = wrapper<type::ident>;
    using cell_list = wrapper<type::list>;
    using cell_func = wrapper<type::function>;

    std::ostream& operator <<(std::ostream &os, cell const &c)
    {
      static int indent_level{ -1 };

      switch(static_cast<type>(c.which()))
      {
        case type::integer:
          os << boost::get<cell_int>(c).data;
          break;
        case type::real:
          os << boost::get<cell_real>(c).data;
          break;
        case type::string:
          os << boost::get<cell_string>(c).data;
          break;
        case type::ident:
          os << "<" << boost::get<cell_ident>(c).data << ">";
          break;
        case type::list:
          ++indent_level;
          os << "\n";
          for(int i{}; i < indent_level; ++i)
          { (void)i; os << "  "; }

          os << "( ";
          for(auto const &v : boost::get<cell_list>(c).data)
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

    namespace detail
    {
      template <type C>
      struct cell_type_variant;
      template <>
      struct cell_type_variant<type::integer>
      { using type = cell_int; };
      template <>
      struct cell_type_variant<type::real>
      { using type = cell_real; };
      template <>
      struct cell_type_variant<type::string>
      { using type = cell_string; };
      template <>
      struct cell_type_variant<type::ident>
      { using type = cell_ident; };
      template <>
      struct cell_type_variant<type::list>
      { using type = cell_list; };
      template <>
      struct cell_type_variant<type::function>
      { using type = cell_func; };
    }
    template <type C>
    using cell_type_variant_t = typename detail::cell_type_variant<C>::type;
  }
}
