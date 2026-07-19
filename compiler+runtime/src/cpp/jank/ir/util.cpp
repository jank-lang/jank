#include <algorithm>

#include <jank/ir/util.hpp>
#include <jank/ir/rewrite.hpp>
#include <jank/ir/processor.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/munge.hpp>

namespace jank::ir
{
  bool uses_name(jtl::ref<instruction> const inst, identifier const &name)
  {
    return rewrite_uses(inst, name, name);
  }

  void replace_with_nop(function &fn, identifier const &block, identifier const &name)
  {
    auto &owning_block{ fn.blocks[fn.find_block(block)] };
    auto const it{ std::ranges::find_if(owning_block.instructions,
                                        [&](auto const &i) { return i->name == name; }) };
    jank_debug_assert(it != owning_block.instructions.end());
    /* TODO: Better name. */
    *it = jtl::make_ref<inst::nop>(runtime::munge(runtime::__rt_ctx->unique_string()));
  }
}
