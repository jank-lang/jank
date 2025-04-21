#include <jank/read/reparse.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/core/meta.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/module/loader.hpp>
#include <jank/util/fmt.hpp>
#include <jank/error/parse.hpp>

namespace jank::read::parse
{
  using namespace jank::runtime;

  static jtl::result<source, error_ref> reparse_nth(jtl::immutable_string const &file_path,
                                                    usize const offset,
                                                    usize const n,
                                                    object_ref const macro_expansion)
  {
    if(file_path == no_source_path)
    {
      return error::internal_parse_failure("Cannot reparse object with no source path.");
    }

    auto const file(module::loader::read_file(file_path));
    if(file.is_err())
    {
      return error::internal_parse_failure(
        util::format("Unable to map file {} due to error: {}", file_path, file.expect_err()));
    }

    lex::processor l_prc{ file.expect_ok().view(), offset };
    parse::processor p_prc{ l_prc.begin(), l_prc.end() };

    auto it{ p_prc.begin() };
    for(usize i{}; i < n; ++i, ++it)
    {
      if(it->is_err())
      {
        return error::internal_parse_failure(util::format("Unable to reparse {} due to error: {}",
                                                          file_path,
                                                          it->expect_err()->message));
      }
    }
    if(it->is_err())
    {
      return error::internal_parse_failure(util::format("Unable to reparse {} due to error: {}",
                                                        file_path,
                                                        it->expect_err()->message));
    }

    auto const &res{ it->expect_ok().unwrap() };
    return source{ file_path, res.start.start, res.end.end, macro_expansion };
  }

  source reparse_nth(obj::persistent_list_ref const o, usize const n)
  {
    auto source(object_source(o));
    if(source == source::unknown)
    {
      return source;
    }

    /* Add one to skip the ( for the list. */
    return reparse_nth(source.file_path, source.start.offset + 1, n, source.macro_expansion)
      .unwrap_move();
  }

  source reparse_nth(runtime::obj::persistent_vector_ref const o, usize const n)
  {
    auto source(object_source(o));
    if(source == source::unknown)
    {
      return source;
    }

    /* Add one to skip the [ for the vector. */
    return reparse_nth(source.file_path, source.start.offset + 1, n, source.macro_expansion)
      .unwrap_move();
  }

  source reparse_nth(runtime::object_ref const o, usize const n)
  {
    /* When we have an object, but we're not sure of the type, let's just
     * see if it's one of the types we support. If not, we'll error out.
     * We can do more here, going forward, by supporting various sequences and such,
     * but this will be fine for now. */
    return visit_seqable(
      [](auto const typed_o, usize const n) -> source {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(std::same_as<T, obj::persistent_list>
                     || std::same_as<T, obj::persistent_vector>)
        {
          return reparse_nth(typed_o, n);
        }
        else
        {
          throw error::internal_parse_failure(
            util::format("Unsupported object for reparsing {}", typed_o->to_code_string()));
        }
      },
      [=]() -> source {
        throw error::internal_parse_failure(
          util::format("Unable to reparse object {}", to_code_string(o)));
      },
      o,
      n);
  }
}
