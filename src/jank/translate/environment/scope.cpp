#include <algorithm>

#include <jank/translate/environment/scope.hpp>
#include <jank/translate/expect/error/type/exception.hpp>
#include <jank/translate/expect/error/internal/exception.hpp>

namespace jank
{
  namespace translate
  {
    namespace environment
    {
      std::experimental::optional<scope::result<cell::type_definition>> scope::find_type
      (std::string const &name) const
      {
        auto const it(type_definitions.find(name));
        if(it == type_definitions.end())
        {
          if(parent)
          { return parent->find_type(name); }
          else
          { return {}; }
        }

        return { { it->second, shared_from_this() } };
      }

      std::experimental::optional<scope::result<cell::binding_definition>> scope::find_binding
      (std::string const &name) const
      {
        auto const it(binding_definitions.find(name));
        if(it == binding_definitions.end())
        {
          if(parent)
          { return parent->find_binding(name); }
          else
          { return {}; }
        }

        return { { it->second, shared_from_this() } };
      }

      template <typename T, typename F>
      static std::experimental::optional<std::vector<scope::result<T>>>
      find
      (
        std::string const &name,
        std::shared_ptr<scope const> const &s,
        std::map<std::string, std::vector<T>> const &functions,
        F const &recurse
      )
      {
        std::vector<scope::result<T>> ret;

        /* Add all from the current scope. */
        auto const it(functions.find(name));
        if(it != functions.end())
        {
          std::transform
          (
            it->second.begin(), it->second.end(),
            std::back_inserter(ret),
            [&](auto const &def) -> scope::result<T>
            { return { def, s }; }
          );
        }

        /* Add all from the parent scope(s). */
        if(s->parent)
        {
          auto const parent_ret((s->parent.get()->*recurse)(name));
          if(parent_ret)
          {
            auto const &found(parent_ret.value());
            std::copy
            (
              found.begin(), found.end(),
              std::back_inserter(ret)
            );
          }
        }

        if(ret.empty())
        { return {}; }
        else
        { return { ret }; }
      }

      /* Build a vector of all function overloads in each scope, from this to the root. */
      std::experimental::optional
      <std::vector<scope::result<cell::function_definition>>>
      scope::find_function(std::string const &name) const
      {
        return find
        (
          name,
          shared_from_this(),
          function_definitions,
          &scope::find_function
        );
      }

      std::experimental::optional
      <std::vector<scope::result<cell::native_function_declaration>>>
      scope::find_native_function(std::string const &name) const
      {
        return find
        (
          name,
          shared_from_this(),
          native_function_declarations,
          &scope::find_native_function
        );
      }

      cell::function_body::type
      scope::expect_function(cell::function_definition::type const &def)
      {
        auto const fail
        (
          [&]
          {
            throw expect::error::internal::exception<>
            { "invalid function definition: " + def.name };
          }
        );

        auto const funcs_opt(find_function(def.name));
        if(!funcs_opt)
        { fail(); }

        auto const funcs(funcs_opt.value());
        auto const found
        (
          std::find_if
          (
            funcs.begin(), funcs.end(),
            [&](auto const &f)
            { return f.first.data.arguments == def.arguments; }
          )
        );
        if(found == funcs.end())
        { fail(); }

        return found->first.data.body;
      }
    }
  }
}
