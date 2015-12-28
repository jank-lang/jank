#pragma once

#include <iostream>
#include <string>
#include <sstream>

namespace jank
{
  /* -- Primitives -- */
  using integer = int64_t;
  using string = std::string;

  /* -- Output -- */
  template <typename T>
  string print_gen_bang_(T const &t)
  {
    std::stringstream ss;
    ss << t;
    std::cout << t.rdbuf() << std::endl;
    return ss.str();
  }

  /* -- Input -- */
  string input_gen_bang_()
  {
    string s;
    std::getline(std::cin, s);
    return s;
  }

  /* -- Primitive arithmetic -- */
  template <typename A, typename B>
  auto _gen_plus_(A const &a, B const &b)
  { return a + b; }

  template <typename A, typename B>
  auto _gen_minus_(A const &a, B const &b)
  { return a - b; }

  template <typename A, typename B>
  auto _gen_asterisk_(A const &a, B const &b)
  { return a * b; }

  auto _gen_asterisk_(string s, integer const n)
  {
    // TODO: Assert n > 0
    auto const original_size(s.size());
    s.reserve(s.size() * n);

    for(integer i{ 1 }; i < n; ++i)
    { s.insert(original_size * i, s.c_str(), original_size); }

    return s;
  }

  template <typename A, typename B>
  auto _gen_slash_(A const &a, B const &b)
  { return a / b; }
}
