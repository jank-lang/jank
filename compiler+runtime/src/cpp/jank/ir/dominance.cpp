#include <jank/ir/processor.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::ir
{
  /* This traverses the CFG to build a reverse post order (RPO) list. We do this
   * by recursively visiting blocks and adding them to the front of the list once
   * we have _finished_ visiting them.
   *
   * We use this RPO list for calculating immediate dominators. */
  void build_rpo(function const &fn,
                 block const &b,
                 native_deque<identifier> &rpo,
                 native_set<identifier> &seen)
  {
    if(seen.contains(b.name))
    {
      return;
    }
    seen.emplace(b.name);

    jank_debug_assert(b.has_terminator());
    auto const terminator{ b.instructions.back() };

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
    switch(terminator->kind)
    {
      case instruction_kind::ret:
      case instruction_kind::throw_:
        break;
      case instruction_kind::jump:
        {
          auto const &next_block_name{ static_cast<inst::jump &>(*terminator.data).block };
          auto &next_block{ fn.blocks[fn.find_block(next_block_name)] };
          build_rpo(fn, next_block, rpo, seen);
        }
        break;
      case instruction_kind::branch:
        {
          auto const &i{ static_cast<inst::branch &>(*terminator.data) };
          auto const &then_block_name{ i.then_block };
          auto &then_block{ fn.blocks[fn.find_block(then_block_name)] };
          build_rpo(fn, then_block, rpo, seen);

          auto const &else_block_name{ i.else_block };
          auto &else_block{ fn.blocks[fn.find_block(else_block_name)] };
          build_rpo(fn, else_block, rpo, seen);

          if(i.merge_block.is_some())
          {
            auto const &merge_block_name{ i.merge_block.unwrap() };
            auto &merge_block{ fn.blocks[fn.find_block(merge_block_name)] };
            build_rpo(fn, merge_block, rpo, seen);
          }
        }
        break;
      case instruction_kind::loop:
        {
          auto const &i{ static_cast<inst::loop &>(*terminator.data) };
          auto const &loop_block_name{ i.loop_block };
          auto &loop_block{ fn.blocks[fn.find_block(loop_block_name)] };
          build_rpo(fn, loop_block, rpo, seen);

          if(i.merge_block.is_some())
          {
            auto const &merge_block_name{ i.merge_block.unwrap() };
            auto &merge_block{ fn.blocks[fn.find_block(merge_block_name)] };
            build_rpo(fn, merge_block, rpo, seen);
          }
        }
        break;
      case instruction_kind::case_:
        {
          auto const &i{ static_cast<inst::case_ &>(*terminator.data) };

          for(auto const &p : i.case_blocks)
          {
            auto const &case_block_name{ p.second };
            auto &case_block{ fn.blocks[fn.find_block(case_block_name)] };
            build_rpo(fn, case_block, rpo, seen);
          }

          auto const &default_block_name{ i.default_block };
          auto &default_block{ fn.blocks[fn.find_block(default_block_name)] };
          build_rpo(fn, default_block, rpo, seen);

          if(i.merge_block.is_some())
          {
            auto const &merge_block_name{ i.merge_block.unwrap() };
            auto &merge_block{ fn.blocks[fn.find_block(merge_block_name)] };
            build_rpo(fn, merge_block, rpo, seen);
          }
        }
        break;
    }
#pragma clang diagnostic pop

    rpo.emplace_front(b.name);
  }

  void build_idom(function &fn, native_deque<identifier> const &rpo)
  {
    jank_debug_assert(1 <= rpo.size());

    /* Map each block name to its RPO index for quick lookup. */
    native_unordered_map<identifier, size_t> rpo_index;
    for(size_t i{}; i < rpo.size(); ++i)
    {
      rpo_index[rpo[i]] = i;
    }

    /* Build predecessor lists from the CFG. */
    native_unordered_map<identifier, native_vector<identifier>> preds;
    for(auto const &b : fn.blocks)
    {
      /* Ensure every block has an entry, even if it has no predecessors. */
      preds.emplace(b.name, native_vector<identifier>{});
    }

    for(auto const &b : fn.blocks)
    {
      jank_debug_assert(b.has_terminator());
      auto const &terminator{ b.instructions.back() };

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
      switch(terminator->kind)
      {
        case instruction_kind::jump:
          {
            auto const &i{ static_cast<inst::jump &>(*terminator.data) };
            preds[i.block].push_back(b.name);
          }
          break;
        case instruction_kind::branch:
          {
            auto const &i{ static_cast<inst::branch &>(*terminator.data) };
            preds[i.then_block].push_back(b.name);
            preds[i.else_block].push_back(b.name);
            if(i.merge_block.is_some())
            {
              preds[i.merge_block.unwrap()].push_back(b.name);
            }
          }
          break;
        case instruction_kind::loop:
          {
            auto const &i{ static_cast<inst::loop &>(*terminator.data) };
            preds[i.loop_block].push_back(b.name);
            if(i.merge_block.is_some())
            {
              preds[i.merge_block.unwrap()].push_back(b.name);
            }
          }
          break;
        case instruction_kind::case_:
          {
            auto const &i{ static_cast<inst::case_ &>(*terminator.data) };
            for(auto const &p : i.case_blocks)
            {
              preds[p.second].push_back(b.name);
            }
            preds[i.default_block].push_back(b.name);
            if(i.merge_block.is_some())
            {
              preds[i.merge_block.unwrap()].push_back(b.name);
            }
          }
          break;
        case instruction_kind::ret:
        case instruction_kind::throw_:
          break;
      }
#pragma clang diagnostic pop
    }

    native_unordered_map<identifier, identifier> idom;
    auto const &entry_name{ rpo.front() };
    idom[entry_name] = entry_name;

    /* The least common denominator (LCD) can be found by climbing the RPO indices. This
     * is a hack around needing to calculate our own depth values per block. */
    auto const lcd{ [&](identifier a, identifier b) -> identifier {
      while(a != b)
      {
        while(rpo_index.at(a) > rpo_index.at(b))
        {
          a = idom.at(a);
        }
        while(rpo_index.at(b) > rpo_index.at(a))
        {
          b = idom.at(b);
        }
      }
      return a;
    } };

    bool changed{ true };
    while(changed)
    {
      changed = false;

      /* We skip entry (index 0). It is its own dominator, by convention. */
      for(size_t i{ 1 }; i < rpo.size(); ++i)
      {
        auto const &block_name{ rpo[i] };
        auto const &block_preds{ preds.at(block_name) };

        /* Find the first predecessor that already has an idom assigned. */
        identifier new_idom{};
        for(auto const &pred : block_preds)
        {
          if(idom.contains(pred))
          {
            new_idom = pred;
            break;
          }
        }

        /* If there is no processed predecessor yet, skip this block for now. */
        if(new_idom.empty())
        {
          continue;
        }

        /* Intersect with all other processed predecessors. */
        for(auto const &pred : block_preds)
        {
          if(pred == new_idom || !idom.contains(pred))
          {
            continue;
          }
          new_idom = lcd(pred, new_idom);
        }

        if(!idom.contains(block_name) || idom.at(block_name) != new_idom)
        {
          idom[block_name] = new_idom;
          changed = true;
        }
      }
    }

    fn.immediate_dominators = jtl::move(idom);
    fn.rpo = jtl::move(rpo);
    fn.block_rpo_index = jtl::move(rpo_index);
  }

  void build_dominance(function &fn)
  {
    native_deque<identifier> rpo;
    native_set<identifier> seen;
    build_rpo(fn, fn.blocks[0], rpo, seen);
    build_idom(fn, rpo);
  }
}
