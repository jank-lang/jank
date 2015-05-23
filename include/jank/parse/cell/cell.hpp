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
        boost::recursive_wrapper<wrapper<type::function>>,
        boost::recursive_wrapper<wrapper<type::comment>>
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
      template <>
      struct wrapper<type::comment>
      {
        using type = std::string;
        type data;
      };

      using boolean = wrapper<type::boolean>;
      using integer = wrapper<type::integer>;
      using real = wrapper<type::real>;
      using string = wrapper<type::string>;
      using ident = wrapper<type::ident>;
      using list = wrapper<type::list>;
      using function = wrapper<type::function>;
      using comment = wrapper<type::comment>;
    }
  }
}
