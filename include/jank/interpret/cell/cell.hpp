#pragma once

#include <string>
#include <vector>
#include <list>
#include <functional>
#include <sstream>
#include <cstdint>

#include <boost/variant.hpp>

#include <jank/parse/cell/cell.hpp>
#include <jank/translate/cell/cell.hpp>
#include <jank/interpret/cell/type.hpp>

namespace jank
{
  namespace interpret
  {
    namespace cell
    {
      template <type C>
      struct wrapper;

      using cell = boost::variant
      <
        boost::recursive_wrapper<wrapper<type::null>>,
        boost::recursive_wrapper<wrapper<type::boolean>>,
        boost::recursive_wrapper<wrapper<type::integer>>,
        boost::recursive_wrapper<wrapper<type::real>>,
        boost::recursive_wrapper<wrapper<type::string>>,
        boost::recursive_wrapper<wrapper<type::function>>,
        boost::recursive_wrapper<wrapper<type::list>>
      >;

      template <>
      struct wrapper<type::null>
      {
        using type = parse::cell::null::type;
        type data;
      };
      template <>
      struct wrapper<type::boolean>
      {
        using type = parse::cell::boolean::type;
        type data;
      };
      template <>
      struct wrapper<type::integer>
      {
        using type = parse::cell::integer::type;
        type data;
      };
      template <>
      struct wrapper<type::real>
      {
        using type = parse::cell::real::type;
        type data;
      };
      template <>
      struct wrapper<type::string>
      {
        using type = parse::cell::string::type;
        type data;
      };
      template <>
      struct wrapper<type::function>
      {
        using type = translate::cell::function_definition::type;
        type data;
      };
      template <>
      struct wrapper<type::list>
      {
        using type = std::list<cell>;
        type data;
      };

      using null = wrapper<type::null>;
      using boolean = wrapper<type::boolean>;
      using integer = wrapper<type::integer>;
      using real = wrapper<type::real>;
      using string = wrapper<type::string>;
      using function = wrapper<type::function>;
      using list = wrapper<type::list>;
    }
  }
}
