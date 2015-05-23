#include <jest/jest.hpp>

#include "parse/paren/match.hpp"
#include "parse/string/escape.hpp"
/* TODO:
 *  numbers,
 *    integers, reals (without tenths place), negative
 *  strings,
 *    escape, utf8
 *  etc.
 */

int main()
{
  jest::worker const j{};
  return j();
}
