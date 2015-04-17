#pragma once

#include <vector>
#include <functional>
#include <sstream>
#include <cstdint>

#include <boost/variant.hpp>

#include <jtl/iterator/range.hpp>

#include <jank/translate/cell/type.hpp>
#include <jank/translate/cell/detail/function_body.hpp>
#include <jank/translate/cell/detail/function_definition.hpp>
#include <jank/translate/cell/detail/function_call.hpp>

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      template <type C>
      struct wrapper;

      using cell = boost::variant
      <
        boost::recursive_wrapper<wrapper<type::function_body>>,
        boost::recursive_wrapper<wrapper<type::function_definition>>,
        boost::recursive_wrapper<wrapper<type::function_call>>
      >;

      template <>
      struct wrapper<type::function_body>
      {
        using type = detail::function_body<cell>;
        type data;
      };
      template <>
      struct wrapper<type::function_definition>
      {
        using type = detail::function_definition<cell>;
        type data;
      };
      template <>
      struct wrapper<type::function_call>
      {
        using type = detail::function_call<cell>;
        type data;
      };

      using function_body = wrapper<type::function_body>;
      using function_definition = wrapper<type::function_definition>;
      using function_call = wrapper<type::function_call>;

      inline std::ostream& operator <<(std::ostream &os, cell const &c)
      {
        static int indent_level{ -1 };

        switch(static_cast<type>(c.which()))
        {
          case type::function_body:
          {
            ++indent_level;
            os << "\n";
            for(auto const i : jtl::it::make_range(0, indent_level))
            { static_cast<void>(i); os << "  "; }

            os << "( ";
            for(auto const &v : boost::get<function_body>(c).data.cells)
            { os << v << " "; }
            os << ") ";

            --indent_level;
          } break;
          case type::function_definition:
          {
            ++indent_level;
            os << "\n";
            for(auto const i : jtl::it::make_range(0, indent_level))
            { static_cast<void>(i); os << "  "; }

            auto const &def(boost::get<function_definition>(c));
            os << "function " << def.data.name << " : " /*<< def.data.arguments*/ << std::endl;
            os << "( ";
            for(auto const &v : def.data.body.cells)
            { os << v << " "; }
            os << ") ";

            --indent_level;
          } break;
          case type::function_call:
          {
            ++indent_level;
            os << "\n";
            for(auto const i : jtl::it::make_range(0, indent_level))
            { static_cast<void>(i); os << "  "; }

            auto const &def(boost::get<function_call>(c));
            os << "call " << def.data.name << " : " /*<< def.data.arguments*/ << std::endl;

            --indent_level;
          } break;
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
        struct type_variant<type::function_body>
        { using type = function_body; };
        template <>
        struct type_variant<type::function_definition>
        { using type = function_definition; };
        template <>
        struct type_variant<type::function_call>
        { using type = function_call; };
      }
      template <type C>
      using type_variant = typename trait::type_variant<C>::type;
    }
  }
}
