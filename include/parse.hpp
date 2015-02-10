#pragma once

#include <regex>

#include <jtl/iterator/range.hpp>

#include <boost/algorithm/string/replace.hpp>

#include "cell.hpp"

cell parse(std::string contents)
{
  cell root{ cell_list{ { cell_ident{ "root" } } } };
  std::vector<cell_list*> list_stack{ &boost::get<cell_list>(root) };

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
        R"(([a-zA-Z\_\+\*\/\^\?\!\:\%\.]+)\s*|)" /* idents */
        R"(\"((?:\\.|[^\\\"])*)\"|)" /* strings */
        R"((\-?\d+(?!\d*\.\d+))|)" /* integers */
        R"((\-?\d+\.\d+))" /* reals */
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
          active_list->data.push_back(cell_list{ { } });
          active_list = &boost::get<cell_list>(active_list->data.back());
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
          active_list->data.push_back(cell_ident{ inner_matches[1] }); }
        else if(inner_matches[2].matched) /* string */
        {
          std::string word(inner_matches[2]);
          boost::algorithm::replace_all(word, "\\\"", "\"");
          std::cout << "string: " << word << std::endl;
          active_list->data.push_back(cell_string{ word });
        }
        else if(inner_matches[3].matched) /* integer */
        {
          std::cout << "int: " << inner_matches[3] << std::endl;
          active_list->data.push_back(cell_int{ std::stoll(inner_matches[3]) }); }
        else if(inner_matches[4].matched) /* real */
        {
          std::cout << "real: " << inner_matches[4] << std::endl;
          active_list->data.push_back(cell_real{ std::stod(inner_matches[4]) });
        }
        else
        { std::cout << "nothing matched" << std::endl; }
      }
    }
  }

  return root;
}
