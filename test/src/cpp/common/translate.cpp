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

namespace jank
{
  namespace common
  {
    jank::translate::cell::function_body translate(std::string const &file)
    {
      /* TODO: Remove duplication of strings. */
      std::ifstream ifs{ "test/src/jank/translate/" + file };
      if(!ifs.is_open())
      { throw std::runtime_error{ "unable to open file: test/src/jank/" + file }; }

      auto root
      (
        jank::parse::parse
        (
          {
            std::istreambuf_iterator<char>{ ifs },
            std::istreambuf_iterator<char>{}
          }
        )
      );

      auto const body(jank::parse::expect::type<jank::parse::cell::type::list>(root));
      auto const scope(std::make_shared<jank::translate::environment::scope>(nullptr));
      auto const translated_body
      (
        jank::translate::translate
        (
          jtl::it::make_range
          (
            std::next(body.data.begin()),
            body.data.end()
          ),
          jank::translate::environment::builtin::type::add_primitives(scope)
        )
      );
      return translated_body;
    }
  }
}
