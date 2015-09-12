#pragma once

#include <vector>
#include <functional>
#include <sstream>
#include <cstdint>

#include <boost/variant.hpp>

#include <jank/translate/cell/type.hpp>
#include <jank/translate/cell/detail/function_body.hpp>
#include <jank/translate/cell/detail/function_definition.hpp>
#include <jank/translate/cell/detail/native_function_definition.hpp>
#include <jank/translate/cell/detail/function_reference.hpp>
#include <jank/translate/cell/detail/native_function_reference.hpp>
#include <jank/translate/cell/detail/function_call.hpp>
#include <jank/translate/cell/detail/indirect_function_call.hpp>
#include <jank/translate/cell/detail/type_definition.hpp>
#include <jank/translate/cell/detail/type_reference.hpp>
#include <jank/translate/cell/detail/binding_definition.hpp>
#include <jank/translate/cell/detail/binding_reference.hpp>
#include <jank/translate/cell/detail/literal_value.hpp>
#include <jank/translate/cell/detail/return_statement.hpp>
#include <jank/translate/cell/detail/if_statement.hpp>
#include <jank/translate/cell/detail/do_statement.hpp>

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
        boost::recursive_wrapper<wrapper<type::native_function_definition>>,
        boost::recursive_wrapper<wrapper<type::function_call>>,
        boost::recursive_wrapper<wrapper<type::indirect_function_call>>,
        boost::recursive_wrapper<wrapper<type::native_function_call>>,
        boost::recursive_wrapper<wrapper<type::function_reference>>,
        boost::recursive_wrapper<wrapper<type::native_function_reference>>,
        boost::recursive_wrapper<wrapper<type::type_definition>>,
        boost::recursive_wrapper<wrapper<type::type_reference>>,
        boost::recursive_wrapper<wrapper<type::binding_definition>>,
        boost::recursive_wrapper<wrapper<type::binding_reference>>,
        boost::recursive_wrapper<wrapper<type::literal_value>>,
        boost::recursive_wrapper<wrapper<type::return_statement>>,
        boost::recursive_wrapper<wrapper<type::if_statement>>,
        boost::recursive_wrapper<wrapper<type::do_statement>>
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
      struct wrapper<type::native_function_definition>
      {
        using type = detail::native_function_definition<cell>;
        type data;
      };
      template <>
      struct wrapper<type::function_call>
      {
        using type = detail::function_call<cell, detail::function_definition<cell>>;
        type data;
      };
      template <>
      struct wrapper<type::indirect_function_call>
      {
        using type = detail::indirect_function_call<cell>;
        type data;
      };
      template <>
      struct wrapper<type::native_function_call>
      {
        using type = detail::function_call<cell, detail::native_function_definition<cell>>;
        type data;
      };
      template <>
      struct wrapper<type::function_reference>
      {
        using type = detail::function_reference<detail::function_definition<cell>>;
        type data;
      };
      template <>
      struct wrapper<type::native_function_reference>
      {
        using type = detail::function_reference<detail::native_function_definition<cell>>;
        type data;
      };
      template <>
      struct wrapper<type::type_definition>
      {
        using type = detail::type_definition<cell>;
        type data;
      };
      template <>
      struct wrapper<type::type_reference>
      {
        using type = detail::type_reference<cell>;
        type data;
      };
      template <>
      struct wrapper<type::binding_definition>
      {
        using type = detail::binding_definition<cell>;
        type data;
      };
      template <>
      struct wrapper<type::binding_reference>
      {
        using type = detail::binding_reference<cell>;
        type data;
      };
      template <>
      struct wrapper<type::literal_value>
      {
        using type = detail::literal_value;
        type data;
      };
      template <>
      struct wrapper<type::return_statement>
      {
        using type = detail::return_statement<cell>;
        type data;
      };
      template <>
      struct wrapper<type::if_statement>
      {
        using type = detail::if_statement<cell>;
        type data;
      };
      template <>
      struct wrapper<type::do_statement>
      {
        using type = detail::do_statement<cell>;
        type data;
      };

      using function_body = wrapper<type::function_body>;
      using function_definition = wrapper<type::function_definition>;
      using native_function_definition = wrapper<type::native_function_definition>;
      using function_call = wrapper<type::function_call>;
      using indirect_function_call = wrapper<type::indirect_function_call>;
      using native_function_call = wrapper<type::native_function_call>;
      using function_reference = wrapper<type::function_reference>;
      using native_function_reference = wrapper<type::native_function_reference>;
      using type_definition = wrapper<type::type_definition>;
      using type_reference = wrapper<type::type_reference>;
      using binding_definition = wrapper<type::binding_definition>;
      using binding_reference = wrapper<type::binding_reference>;
      using literal_value = wrapper<type::literal_value>;
      using return_statement = wrapper<type::return_statement>;
      using if_statement = wrapper<type::if_statement>;
      using do_statement = wrapper<type::do_statement>;
    }
  }
}
