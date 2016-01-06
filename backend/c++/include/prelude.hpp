#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <functional>

namespace jank
{
  /* -- Primitives -- */
  using integer = int64_t;
  using real = double;
  using boolean = bool;
  using string = std::string;

  /* -- Assertions -- */
  void assert_gen_bang_(boolean const b, string const &s)
  {
    if(!b)
    { throw std::runtime_error{ "(assertion failure) " + s }; }
  }
  void assert_gen_minus_not_gen_bang_(boolean const b)
  { assert_gen_bang_(!b, ""); }
  void assert_gen_minus_not_gen_bang_(boolean const b, string const &s)
  { assert_gen_bang_(!b, s); }
  void assert_gen_minus_unreachable_gen_bang_()
  { assert_gen_bang_(false, "unreachable code reached"); }

  /* -- Output -- */
  template <typename T>
  string print_gen_bang_(T const &t)
  {
    std::stringstream ss;
    ss << t;
    std::cout << ss.rdbuf() << std::endl;
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

  auto operator *(string s, integer const n)
  {
    assert_gen_bang_(n > 0, "invalid scalar for string repetition");
    auto const original_size(s.size());
    s.reserve(s.size() * n);

    for(integer i{ 1 }; i < n; ++i)
    { s.insert(original_size * i, s.c_str(), original_size); }

    return s;
  }

  template <typename A, typename B>
  auto _gen_asterisk_(A const &a, B const &b)
  { return a * b; }

  template <typename A, typename B>
  auto _gen_slash_(A const &a, B const &b)
  { return a / b; }
}
