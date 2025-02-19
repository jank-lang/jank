#include <fmt/format.h>

#include <jank/native_persistent_string/fmt.hpp>
#include <jank/read/reparse.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/obj/persistent_list.hpp>
#include <jank/runtime/core/meta.hpp>
#include <jank/util/mapped_file.hpp>
#include <jank/error/parse.hpp>

namespace jank::read::parse
{
  using namespace jank::runtime;

  /* TODO: How does macro expansion fall into this? Will we not be able to reparse some things? */
  static result<source, error_ptr>
  reparse_nth(native_persistent_string const &file_path, size_t const offset, size_t const n)
  {
    if(file_path == no_source_path)
    {
      return error::internal_parse_failure("Cannot reparse object with no source path");
    }

    /* TODO: JAR support */
    auto const file(util::map_file({ file_path.data(), file_path.size() }));
    if(file.is_err())
    {
      return error::internal_parse_failure(
        fmt::format("Unable to map file {} due to error: {}", file_path, file.expect_err()));
    }

    lex::processor l_prc{
      { file.expect_ok().head, file.expect_ok().size },
      offset
    };
    parse::processor p_prc{ l_prc.begin(), l_prc.end() };

    auto it{ p_prc.begin() };
    for(size_t i{}; i < n; ++i, ++it)
    {
      if(it->is_err())
      {
        return error::internal_parse_failure(fmt::format("Unable to reparse {} due to error: {}",
                                                         file_path,
                                                         it->expect_err()->message));
      }
    }
    if(it->is_err())
    {
      return error::internal_parse_failure(
        fmt::format("Unable to reparse {} due to error: {}", file_path, it->expect_err()->message));
    }

    auto const &res{ it->expect_ok().unwrap() };
    return source{ file_path, res.start.start, res.end.end };
  }

  source reparse_nth(obj::persistent_list_ptr const l, size_t const n)
  {
    auto const source(meta_source(l->meta));
    if(source == source::unknown)
    {
      throw error::internal_parse_failure(
        fmt::format("Unknown source while trying to reparse {}", l->to_code_string()));
    }

    /* Add one to skip the ( for the list. */
    return reparse_nth(source.file_path, source.start.offset + 1, n).unwrap_move();
  }
}
