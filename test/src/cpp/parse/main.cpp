#include <jest/jest.hpp>

#include "parse/paren/match.hpp"
#include "parse/string/escape.hpp"
#include "parse/comment/single_line.hpp"
#include "parse/comment/multi_line.hpp"
#include "parse/comment/nested.hpp"
#include "parse/ident/ascii.hpp"
#include "parse/ident/unicode.hpp"

/* TODO:
 *  numbers,
 *    integers, reals (without tenths place), negative
 *  strings,
 *    utf8
 *  etc.
 */

int main()
{
  jest::worker const j{};
  return j();
}
