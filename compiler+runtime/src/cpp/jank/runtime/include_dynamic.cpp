#include <jank/util/fmt.hpp>

#include <jank/runtime/context.hpp>
#include <jank/runtime/include.hpp>
#include <jank/runtime/obj/persistent_hash_map.hpp>
#include <jank/runtime/obj/persistent_list.hpp>
#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/obj/symbol.hpp>

namespace jank::runtime
{
  static obj::persistent_list_ref cpp_raw_include(jtl::immutable_string const &header)
  {
    static auto const raw(make_box<obj::symbol>("cpp/raw"));

    auto const cpp(header.starts_with("./") ? util::format("#include \"{}\"", header)
                                            : util::format("#include <{}>", header));

    return make_box<obj::persistent_list>(std::in_place,
                                          raw,
                                          make_box<obj::persistent_string>(cpp));
  }

  obj::persistent_list_ref include_header(jtl::immutable_string const &header)
  {
    static auto const _do(make_box<obj::symbol>("do"));
    static auto const load_header_symbol(make_box<obj::symbol>("cpp/jank.runtime.load_header"));

    auto const load_header_form(
      make_box<obj::persistent_list>(std::in_place,
                                     load_header_symbol,
                                     make_box<obj::persistent_string>(header)));

    return make_box<obj::persistent_list>(std::in_place,
                                          _do,
                                          cpp_raw_include(header),
                                          load_header_form);
  }

  void load_header(jtl::immutable_string const &header)
  {
    context::binding_scope const _{ obj::persistent_hash_map::create_unique(
      std::make_pair(__rt_ctx->compile_files_var, jank_false)) };

    auto const cpp(header.starts_with("./") ? util::format("#include \"{}\"", header)
                                            : util::format("#include <{}>", header));

    __rt_ctx->eval_cpp_string(cpp).expect_ok();
  }
}
