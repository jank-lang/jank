#include <jank/read/reparse.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/to_string.hpp>
#include <jank/runtime/core/meta.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/module/loader.hpp>
#include <jank/util/fmt.hpp>
#include <jank/error/parse.hpp>

namespace jank::read::parse
{
  using namespace jank::runtime;

  static jtl::result<source, error_ref> reparse_nth(jtl::immutable_string const &file,
                                                    jtl::immutable_string const &module,
                                                    usize const offset,
                                                    usize const n,
                                                    object_ref const macro_expansion)
  {
    if(module == no_source_path)
    {
      return error::internal_parse_failure("Cannot reparse object with no source path.");
    }

    auto mapped_file(runtime::module::loader::read_file(file));
    if(mapped_file.is_err())
    {
      if(file != read::no_source_path)
      {
        mapped_file = runtime::__rt_ctx->module_loader.read_module(module);
      }
      if(mapped_file.is_err())
      {
        return error::internal_parse_failure(
          util::format("Unable to map module '{}' due to error '{}'.",
                       module,
                       mapped_file.expect_err()));
      }
    }

    lex::processor l_prc{ mapped_file.expect_ok().view(), offset };
    parse::processor p_prc{ l_prc.begin(), l_prc.end() };

    auto it{ p_prc.begin() };
    for(usize i{}; i < n; ++i, ++it)
    {
      if(it->is_err())
      {
        return error::internal_parse_failure(
          util::format("Unable to reparse module '{}' due to error '{}'.",
                       module,
                       it->expect_err()->message));
      }
    }
    if(it->is_err())
    {
      return error::internal_parse_failure(
        util::format("Unable to reparse module '{}' due to error '{}'.",
                     module,
                     it->expect_err()->message));
    }

    auto const &res{ it->expect_ok().unwrap() };
    return source{ file, module, res.start.start, res.end.end, macro_expansion };
  }

  source reparse_nth(obj::persistent_list_ref const o, usize const n)
  {
    auto source(object_source(o));
    if(source == source::unknown)
    {
      return source;
    }

    /* Add one to skip the ( for the list. */
    auto const res{
      reparse_nth(source.file, source.module, source.start.offset + 1, n, source.macro_expansion)
    };
    if(res.is_err())
    {
      return source::unknown;
    }
    return res.expect_ok();
  }

  source reparse_nth(runtime::obj::persistent_vector_ref const o, usize const n)
  {
    auto source(object_source(o));
    if(source == source::unknown)
    {
      return source;
    }

    /* Add one to skip the [ for the vector. */
    auto const res{
      reparse_nth(source.file, source.module, source.start.offset + 1, n, source.macro_expansion)
    };
    if(res.is_err())
    {
      return source::unknown;
    }
    return res.expect_ok();
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
            util::format("Unsupported object for reparsing '{}'.", typed_o->to_code_string()));
        }
      },
      [=]() -> source {
        throw error::internal_parse_failure(
          util::format("Unable to reparse object '{}'.", to_code_string(o)));
      },
      o,
      n);
  }
}
