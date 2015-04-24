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
#include <jank/translate/cell/detail/variable_definition.hpp>
#include <jank/translate/cell/detail/variable_reference.hpp>
#include <jank/translate/cell/detail/literal_value.hpp>

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
        boost::recursive_wrapper<wrapper<type::function_call>>,
        boost::recursive_wrapper<wrapper<type::variable_definition>>,
        boost::recursive_wrapper<wrapper<type::variable_reference>>,
        boost::recursive_wrapper<wrapper<type::literal_value>>
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
      template <>
      struct wrapper<type::variable_definition>
      {
        using type = detail::variable_definition;
        type data;
      };
      template <>
      struct wrapper<type::variable_reference>
      {
        using type = detail::variable_reference;
        type data;
      };
      template <>
      struct wrapper<type::literal_value>
      {
        using type = detail::literal_value;
        type data;
      };

      using function_body = wrapper<type::function_body>;
      using function_definition = wrapper<type::function_definition>;
      using function_call = wrapper<type::function_call>;
      using variable_definition = wrapper<type::variable_definition>;
      using variable_reference = wrapper<type::variable_reference>;
      using literal_value = wrapper<type::literal_value>;
    }
  }
}
