#pragma once

#include <regex>

#include <boost/algorithm/string/replace.hpp>

#include <jtl/iterator/range.hpp>

#include <jank/interpret/cell/cell.hpp>
#include <jank/interpret/expect/type.hpp>

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
            R"(([a-zA-Z\-\+\*\/\^\?\!\:\%\.]+)(?:\s|\$)*)" /* idents */
            R"(|\"((?:\\.|[^\\\"])*)\")" /* strings */
            R"(|(\-?\d+(?!\d*\.\d+)))" /* integers */
            R"(|(\-?\d+\.\d+))" /* reals */
          };
          auto const &outer_str(outer_match.str());
          if(outer_str.empty())
          { continue; }

          auto active_list(list_stack.back());
          if(outer_str[0] == '(')
          {
            std::cout << outer_str << std::endl;
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
            std::cout << outer_str << std::endl;
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
            {
              std::cout << "ident: " << inner_matches[1] << std::endl;
              active_list->data.push_back(cell::ident{ inner_matches[1] });
            }
            else if(inner_matches[2].matched) /* string */
            {
              std::string word(inner_matches[2]);
              boost::algorithm::replace_all(word, "\\\"", "\"");
              std::cout << "string: " << word << std::endl;
              active_list->data.push_back(cell::string{ word });
            }
            else if(inner_matches[3].matched) /* integer */
            {
              std::cout << "integer: " << inner_matches[3] << std::endl;
              active_list->data.push_back(cell::integer{ std::stoll(inner_matches[3]) });
            }
            else if(inner_matches[4].matched) /* real */
            {
              std::cout << "real: " << inner_matches[4] << std::endl;
              active_list->data.push_back(cell::real{ std::stod(inner_matches[4]) });
            }
            else
            { std::cout << "nothing matched" << std::endl; }
          }
        }
      }

      return root;
    }
  }
}
