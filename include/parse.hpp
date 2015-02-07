#pragma once

#include "cell.hpp"

cell parse(std::string contents)
{
  cell root{ cell_list{ { cell_string{ "root" } } } };
  std::vector<cell_list*> list_stack{ &boost::get<cell_list>(root) };

  for(auto it(contents.begin()); it != contents.end();)
  {
    auto const index(std::distance(contents.begin(), it));
    auto const next(contents.find_first_not_of(" \t\n\r\v", index));
    if(next == std::string::npos)
    { break; }

    auto const word_end(contents.find_first_of(" \t\n\r\v", next));
    it = std::next(contents.begin(), word_end);

    auto word(contents.substr(next, word_end - next));
    if(word.empty())
    { continue; }

    auto active_list(list_stack.back());
    auto const starts_list(word.find_first_not_of("("));
    auto const started(starts_list == std::string::npos ? 0 : starts_list);
    auto const ends_list(word.find_last_not_of(")"));
    auto const ended(ends_list == word.size() - 1 ? 0 : word.size() - ends_list - 1);
    std::cout << "word: '" << word << "'\n\t"
              << "([starts_list = " << starts_list << ", "
              << "started = " << started << "], "
              << "[ends_list = " << ends_list << ", "
              << "ended = " << ended << "])"
              << std::endl;
    if(started)
    { word.erase(0, started); }
    if(ended)
    { word.erase(ends_list + 1); }

    cell word_cell{ cell_string{ word } };
    auto const is_func(env.cells.find(word));
    if(is_func != env.cells.end())
    { }
    else if(std::isdigit(word[0]) || word[0] == '-')
    { word_cell = cell_int{ std::stoll(word) }; }

    if(started)
    {
      for(std::size_t i{}; i < started; ++i)
      {
        active_list->data.push_back(cell_list{ { } });
        active_list = &boost::get<cell_list>(active_list->data.back());
        list_stack.push_back(active_list);
      }
      active_list->data.push_back(word_cell);
    }
    else
    { active_list->data.push_back(word_cell); }

    for(std::size_t i{}; i < ended; ++i)
    { list_stack.pop_back(); }
  }
  return root;
}
