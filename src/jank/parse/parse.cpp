#include <regex>

#include <boost/algorithm/string/replace.hpp>

#include <jtl/iterator/range.hpp>

#include <jank/parse/parse.hpp>
#include <jank/interpret/environment/scope.hpp> /* TODO: shouldn't use interpret here */
#include <jank/parse/expect/type.hpp>

namespace jank
{
  namespace parse
  {
    cell::cell parse(std::string contents)
    {
      cell::cell root{ cell::list{ { cell::ident{ "root" } } } };
      std::vector<cell::list*> list_stack{ &expect::type<cell::type::list>(root) };

      static std::regex outer_regex{ R"((\(*)([^\)\(]*)(\)*))" };
      std::sregex_iterator const outer_begin
      { contents.begin(), contents.end(), outer_regex };
      std::sregex_iterator const end{};

      for(auto const &outer : jtl::it::make_range(outer_begin, end))
      {
        std::smatch const &outer_matches{ outer };
        for
        (
          auto const &outer_match :
          jtl::it::make_range(std::next(outer_matches.begin()), outer_matches.end())
        )
        {
          static std::regex inner_regex
          {
            /* TODO: _ in ident */
            R"(([a-zA-Z\-\+\*\/\^\?\!\:\%\.]+))" /* idents */
            R"(|\"((?:\\.|[^\\\"])*)\")" /* strings */
            R"(|(\-?\d+(?!\d*\.\d+)))" /* integers */
            R"(|(\-?\d+\.\d+))" /* reals */
            R"(|(true|false))" /* booleans */
            /* XXX: A GCC bug in 4.9 causes this to be read right to left.
             * This will hopefully be fixed in GCC 5.0; it works fine in clang
             * but I'm targetting GCC for now. */
          };
          auto const &outer_str(outer_match.str());
          if(outer_str.empty())
          { continue; }

          auto active_list(list_stack.back());
          if(outer_str[0] == '(')
          {
            for(auto const &open : outer_str)
            {
              static_cast<void>(open);
              active_list->data.push_back(cell::list{ { } });
              active_list = &expect::type<cell::type::list>
              (active_list->data.back());
              list_stack.push_back(active_list);
            }
            continue;
          }
          else if(outer_str[0] == ')')
          {
            for(auto const &close : outer_str)
            {
              static_cast<void>(close);
              list_stack.pop_back();
            }
            continue;
          }

          std::sregex_iterator const inner_begin
          { outer_str.begin(), outer_str.end(), inner_regex };
          for(auto const &inner : jtl::it::make_range(inner_begin, end))
          {
            std::smatch const &inner_matches{ inner };
            if(inner_matches[1].matched) /* ident */
            { active_list->data.push_back(cell::ident{ inner_matches[1] }); }
            else if(inner_matches[2].matched) /* string */
            {
              std::string word(inner_matches[2]);
              boost::algorithm::replace_all(word, "\\\"", "\"");
              active_list->data.push_back(cell::string{ word });
            }
            else if(inner_matches[3].matched) /* integer */
            { active_list->data.push_back(cell::integer{ std::stoll(inner_matches[3]) }); }
            else if(inner_matches[4].matched) /* real */
            { active_list->data.push_back(cell::real{ std::stod(inner_matches[4]) }); }
            else if(inner_matches[5].matched) /* boolean */
            { active_list->data.push_back(cell::boolean{ inner_matches[5] == "true" }); }
            else
            { throw std::runtime_error{ "invalid parsing match" }; }
          }
        }
      }

      return root;
    }
  }
}
