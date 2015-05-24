#include <jest/jest.hpp>

#include "parse/paren/match.hpp"
#include "parse/string/escape.hpp"
#include "parse/comment/single_line.hpp"
#include "parse/comment/multi_line.hpp"
//#include "parse/comment/nested.hpp"
/* TODO:
 *  comments!
 *
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
