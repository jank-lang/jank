#include <algorithm>

#include <jank/runtime/context.hpp>
#include <jank/runtime/core/munge.hpp>
#include <jank/ir/processor.hpp>
#include <jank/ir/rewrite.hpp>
#include <jank/ir/util.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::ir
{
  struct var_deref_info
  {
    native_unordered_map<jtl::immutable_string, native_vector<std::pair<identifier, identifier>>>
      derefs;
    bool has_loop{};
  };

  /* Build a map from qualified var to (block name, instruction name). */
  static var_deref_info collect_var_derefs(function const &fn)
  {
    var_deref_info result;

    for(auto const &block : fn.blocks)
    {
      for(auto const &instr : block.instructions)
      {
        if(instr->kind == instruction_kind::loop)
        {
          /* Loops invalidate this optimization, so we're done collecting. */
          result.has_loop = true;
          return result;
        }

        if(instr->kind != instruction_kind::var_deref)
        {
          continue;
        }

        auto const &lit{ static_cast<inst::var_deref &>(*instr.data) };

        /* Dynamic vars need to be handled more carefully, so we don't hoist them. */
        if(runtime::__rt_ctx->intern_var(lit.qualified_var).expect_ok()->dynamic.load())
        {
          continue;
        }

        result.derefs[lit.qualified_var].emplace_back(block.name, instr->name);
      }
    }

    return result;
  }

  void hoist_var_derefs(function &fn)
  {
    jank_debug_assert(!fn.immediate_dominators.empty());

    if(fn.name.starts_with("jank_load_"))
    {
      return;
    }

    auto const &idom{ fn.immediate_dominators };
    auto const var_derefs{ collect_var_derefs(fn) };

    /* TODO: If a loop is present, hoist within the loop. Change collection to work on a set
     * of blocks, rather than a fn. Hoist within each loop. */
    if(var_derefs.has_loop)
    {
      return;
    }

    for(auto const &[qualified_var, occurrences] : var_derefs.derefs)
    {
      /* Nothing to deduplicate for a unique occurrence. */
      if(occurrences.size() == 1)
      {
        continue;
      }

      /* Collect just the block names for lcd. */
      native_vector<identifier> owning_blocks;
      owning_blocks.reserve(occurrences.size());
      for(auto const &[block_name, _] : occurrences)
      {
        owning_blocks.push_back(block_name);
      }

      auto const target_block_name{ lcd(idom, fn.block_rpo_index, owning_blocks) };
      auto &target_block{ fn.blocks[fn.find_block(target_block_name)] };

      /* Check whether the target block already has a canonical literal for
       * this value. If so, reuse it; otherwise we need to insert one. */
      identifier canonical_name{};
      for(auto const &[block_name, instr_name] : occurrences)
      {
        if(block_name == target_block_name)
        {
          canonical_name = instr_name;
          break;
        }
      }

      if(canonical_name.empty())
      {
        /* Generate a fresh name and prepend a new literal into the target
         * block, after any leading parameter and capture instructions. */
        /* TODO: Move logic for new idents from builder into function. Use it here. */
        canonical_name = runtime::munge(runtime::__rt_ctx->unique_string());

        auto insert_pos{ target_block.instructions.begin() };
        while(insert_pos != target_block.instructions.end()
              && ((*insert_pos)->kind == instruction_kind::parameter
                  || (*insert_pos)->kind == instruction_kind::capture))
        {
          ++insert_pos;
        }

        /* Copy type info from any of the existing occurrences — they all
         * produce the same value so the type is identical. */
        auto const &existing_instr_name{ occurrences[0].second };
        auto &existing_block{ fn.blocks[fn.find_block(occurrences[0].first)] };
        auto const existing_it{ std::ranges::find_if(
          existing_block.instructions,
          [&](auto const &i) { return i->name == existing_instr_name; }) };
        jank_debug_assert(existing_it != existing_block.instructions.end());

        auto const &existing{ dynamic_cast<inst::var_deref &>(*(*existing_it).data) };

        auto const new_instr{
          jtl::make_ref<inst::var_deref>(canonical_name, existing.location, existing.qualified_var)
        };

        target_block.instructions.insert(insert_pos, new_instr);
      }

      /* Nop out every occurrence that isn't the canonical one and rewrite
       * all uses across the whole function to point to canonical_name. */
      for(auto const &occurrence : occurrences)
      {
        if(occurrence.second == canonical_name)
        {
          continue;
        }

        rewrite_uses(fn, occurrence.second, canonical_name);

        replace_with_nop(fn, occurrence.first, occurrence.second);
      }
    }
  }
}
