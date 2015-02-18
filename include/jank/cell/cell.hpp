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
    template <cell_type C>
    struct cell_wrapper;

    using cell = boost::variant
    <
      boost::recursive_wrapper<cell_wrapper<cell_type::integer>>,
      boost::recursive_wrapper<cell_wrapper<cell_type::real>>,
      boost::recursive_wrapper<cell_wrapper<cell_type::string>>,
      boost::recursive_wrapper<cell_wrapper<cell_type::ident>>,
      boost::recursive_wrapper<cell_wrapper<cell_type::list>>,
      boost::recursive_wrapper<cell_wrapper<cell_type::function>>
    >;

    template <>
    struct cell_wrapper<cell_type::integer>
    {
      using type = int64_t;
      type data;
    };
    template <>
    struct cell_wrapper<cell_type::real>
    {
      using type = double;
      type data;
    };
    template <>
    struct cell_wrapper<cell_type::string>
    {
      using type = std::string;
      type data;
    };
    template <>
    struct cell_wrapper<cell_type::ident>
    {
      using type = std::string;
      type data;
    };
    template <>
    struct cell_wrapper<cell_type::list>
    {
      using type = std::vector<cell>;
      type data;
    };

    using cell_int = cell_wrapper<cell_type::integer>;
    using cell_real = cell_wrapper<cell_type::real>;
    using cell_string = cell_wrapper<cell_type::string>;
    using cell_ident = cell_wrapper<cell_type::ident>;
    using cell_list = cell_wrapper<cell_type::list>;
    using cell_func = cell_wrapper<cell_type::function>;

    std::ostream& operator <<(std::ostream &os, cell const &c)
    {
      static int indent_level{ -1 };

      switch(static_cast<cell_type>(c.which()))
      {
        case cell_type::integer:
          os << boost::get<cell_int>(c).data;
          break;
        case cell_type::real:
          os << boost::get<cell_real>(c).data;
          break;
        case cell_type::string:
          os << boost::get<cell_string>(c).data;
          break;
        case cell_type::ident:
          os << "<" << boost::get<cell_ident>(c).data << ">";
          break;
        case cell_type::list:
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
        case cell_type::function:
          os << "function ";
          break;
        default:
          os << "??? ";
      }

      return os;
    }

    namespace detail
    {
      template <cell_type C>
      struct cell_type_variant;
      template <>
      struct cell_type_variant<cell_type::integer>
      { using type = cell_int; };
      template <>
      struct cell_type_variant<cell_type::real>
      { using type = cell_real; };
      template <>
      struct cell_type_variant<cell_type::string>
      { using type = cell_string; };
      template <>
      struct cell_type_variant<cell_type::ident>
      { using type = cell_ident; };
      template <>
      struct cell_type_variant<cell_type::list>
      { using type = cell_list; };
      template <>
      struct cell_type_variant<cell_type::function>
      { using type = cell_func; };
    }
    template <cell_type C>
    using cell_type_variant_t = typename detail::cell_type_variant<C>::type;
  }
}
