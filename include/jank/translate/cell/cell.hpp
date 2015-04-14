#pragma once

#include <vector>
#include <functional>
#include <sstream>
#include <cstdint>

#include <boost/variant.hpp>

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
