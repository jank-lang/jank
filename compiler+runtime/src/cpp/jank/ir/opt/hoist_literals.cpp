#include <algorithm>

#include <jank/runtime/context.hpp>
#include <jank/runtime/core/munge.hpp>
#include <jank/ir/processor.hpp>
#include <jank/ir/rewrite.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::ir
{
  /* Collect all literal instructions in the function, grouped by their value.
   * Returns a map of: literal value to list of (block name, instruction name) */
  static native_unordered_map<runtime::object_ref,
                              native_vector<std::pair<identifier, identifier>>,
                              std::hash<runtime::object_ref>,
                              runtime::very_equal_to_with_meta>
  collect_literals(function const &fn)
  {
    native_unordered_map<runtime::object_ref,
                         native_vector<std::pair<identifier, identifier>>,
                         std::hash<runtime::object_ref>,
                         runtime::very_equal_to_with_meta>
      result;

    for(auto const &block : fn.blocks)
    {
      for(auto const &instr : block.instructions)
      {
        if(instr->kind != instruction_kind::literal)
        {
          continue;
        }
        auto const &lit{ static_cast<inst::literal &>(*instr.data) };
        result[lit.obj].emplace_back(block.name, instr->name);
      }
    }

    return result;
  }

  /* Hoist and deduplicate literal instructions across the function.
   *
   * For each unique literal value:
   *   1. Collect all blocks containing a :literal for that value
   *   2. Find the LCD of those blocks in the dominator tree
   *   3. Ensure exactly one canonical :literal lives in the LCD block,
   *      prepended before any non-parameter instructions
   *   4. Rewrite all uses of the duplicate literals to the canonical name
   *   5. Replace the duplicate :literal instructions with :nop
   *
   * This pass requires dominance to already be built on `fn`. */
  void hoist_literals(function &fn)
  {
    jank_debug_assert(!fn.immediate_dominators.empty());

    auto const &idom{ fn.immediate_dominators };
    auto const literals{ collect_literals(fn) };

    for(auto const &[value, occurrences] : literals)
    {
      /* Nothing to deduplicate for a unique literal. */
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

        auto const &existing_lit{ dynamic_cast<inst::literal &>(*(*existing_it).data) };

        auto const new_instr{ jtl::make_ref<inst::literal>(canonical_name,
                                                           existing_lit.type,
                                                           existing_lit.location,
                                                           existing_lit.obj,
                                                           existing_lit.value) };

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

        auto &owning_block{ fn.blocks[fn.find_block(occurrence.first)] };
        auto const it{ std::ranges::find_if(owning_block.instructions, [&](auto const &i) {
          return i->name == occurrence.second;
        }) };
        jank_debug_assert(it != owning_block.instructions.end());
        /* TODO: Better name. */
        *it = jtl::make_ref<inst::nop>(runtime::munge(runtime::__rt_ctx->unique_string()));
      }
    }
  }
}
