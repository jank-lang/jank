#pragma once

#include <string>
#include <stdexcept>

#include <jank/interpret/expect/error/type/type.hpp>

namespace jank
{
  namespace translate
  {
    namespace cell
    {
      enum class type
      {
        body,
        function_definition,
        function_call
      };

      template <type C>
      char constexpr const* type_string();
      template <>
      inline char constexpr const* type_string<type::body>()
      { return "body"; }
      template <>
      inline char constexpr const* type_string<type::function_definition>()
      { return "function_definition"; }
      template <>
      inline char constexpr const* type_string<type::function_call>()
      { return "function_call"; }

      /* TODO: constexpr */
      inline char const* type_string(type const c)
      {
        switch(c)
        {
          case type::body:
            return type_string<type::body>();
          case type::function_definition:
            return type_string<type::function_definition>();
          case type::function_call:
            return type_string<type::function_call>();
          default:
            return "unknown";
        }
      }

      /* TODO: This should be in env. */
      inline type type_from_string(std::string const &str)
      {
        if(str == "body")
        { return type::body; }
        else if(str == "function_definition")
        { return type::function_definition; }
        else if(str == "function_call")
        { return type::function_call; }
        else
        {
          throw interpret::expect::error::type::type<>
          { "invalid translate cell type string " + str };
        }
      }
    }
  }
}
