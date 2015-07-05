#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <fstream>
#include <algorithm>

#include <jtl/iterator/stream_delim.hpp>
#include <jtl/iterator/back_insert.hpp>

#include "common/translate.hpp"

#include <jank/parse/parse.hpp>
#include <jank/parse/cell/stream.hpp>
#include <jank/parse/expect/type.hpp>
#include <jank/translate/translate.hpp>
#include <jank/translate/expect/type.hpp>
#include <jank/translate/cell/stream.hpp>
#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/plugin/apply.hpp>

namespace jank
{
  namespace common
  {
    std::pair<parse::cell::cell, translate::cell::function_body>
    translate(std::string const &file)
    {
      static std::string const prefix("test/src/jank/");

      std::ifstream ifs{ prefix + file };
      if(!ifs.is_open())
      { throw std::runtime_error{ "unable to open file: " + prefix + file }; }

      auto root
      (
        parse::parse
        (
          {
            std::istreambuf_iterator<char>{ ifs },
            std::istreambuf_iterator<char>{}
          }
        )
      );

      auto const body(parse::expect::type<parse::cell::type::list>(root));
      auto const scope(std::make_shared<translate::environment::scope>(nullptr));
      translate::environment::builtin::type::add_primitives(*scope);
      translate::plugin::apply(scope);
      auto const translated_body
      (
        translate::translate
        (
          jtl::it::make_range
          (
            std::next(body.data.begin()),
            body.data.end()
          ),
          scope,
          { /* The outermost body returns null. */
            translate::environment::builtin::type::null(*scope)
          }
        )
      );
      return { root, translated_body };
    }
  }
}
