#include <jank/runtime/context.hpp>
#include <jank/runtime/core/munge.hpp>
#include <jank/analyze/cpp_util.hpp>
#include <jank/ir/processor.hpp>
#include <jank/ir/rewrite.hpp>
#include <jank/ir/util.hpp>
#include <jank/ir/walk.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::ir
{
  struct scope_info
  {
    jtl::ptr<instruction> inst{};
    native_deque<identifier> definition_scope_stack{};
    identifier definition_block{};
    native_deque<identifier> lowest_used_scope_stack{};
    identifier lowest_used_block{};
  };

  struct collection
  {
    native_unordered_map<identifier, scope_info> locals;
    native_unordered_map<identifier, identifier> scope_to_block;
  };

  static identifier top_scope(native_deque<identifier> const &scope_stack)
  {
    if(scope_stack.empty())
    {
      return {};
    }
    return scope_stack.back();
  }

  //static bool does_left_scope_stack_dominate(native_deque<identifier> const &lhs,
  //                                           native_deque<identifier> const &rhs)
  //{
  //  if(rhs.size() < lhs.size())
  //  {
  //    return false;
  //  }

  //  for(usize i{}; i != lhs.size(); ++i)
  //  {
  //    if(lhs[i] != rhs[i])
  //    {
  //      return false;
  //    }
  //  }

  //  return true;
  //}

  static identifier
  lcd_scope(native_deque<identifier> const &lhs, native_deque<identifier> const &rhs)
  {
    jank_debug_assert(!lhs.empty());
    jank_debug_assert(!rhs.empty());

    for(usize i{}; i < lhs.size(); ++i)
    {
      if(rhs.size() <= i)
      {
        return lhs[i - 1];
      }
      if(lhs[i] != rhs[i])
      {
        return lhs[i - 1];
      }
    }
    return lhs.back();
  }

  // def: s0 s1
  // lhs: s0 s1 s2
  // rhs: s0 s3
  static bool is_scope_stack_less_nested(native_deque<identifier> const &def,
                                         native_deque<identifier> const &lhs,
                                         native_deque<identifier> const &rhs)
  {
    /* NOLINTNEXTLINE(readability-suspicious-call-argument) */
    auto const &lhs_lcd{ lcd_scope(def, lhs) };
    auto const &rhs_lcd{ lcd_scope(def, rhs) };
    return std::stoi(lhs_lcd.substr(1)) < std::stoi(rhs_lcd.substr(1));
  }

  static bool is_scope_stack_less_nested_or_equal(native_deque<identifier> const &def,
                                                  native_deque<identifier> const &lhs,
                                                  native_deque<identifier> const &rhs)
  {
    /* NOLINTNEXTLINE(readability-suspicious-call-argument) */
    auto const &lhs_lcd{ lcd_scope(def, lhs) };
    auto const &rhs_lcd{ lcd_scope(def, rhs) };
    return std::stoi(lhs_lcd.substr(1)) <= std::stoi(rhs_lcd.substr(1));
  }

  static collection collect_locals(function const &fn)
  {
    collection ret;
    native_deque<identifier> scope_stack;

    walk(fn, [&](jtl::ref<instruction> const instr, identifier const &block) {
      if(instr->kind == instruction_kind::cpp_scope_open)
      {
        auto const &i{ static_cast<inst::cpp_scope_open &>(*instr.data) };
        scope_stack.push_back(i.name);
        ret.scope_to_block.emplace(i.name, block);
        //util::println("entering scope {}", i.name);
        return;
      }
      else if(instr->kind == instruction_kind::cpp_scope_close)
      {
        //util::println("exiting scope {}", scope_stack.back());
        scope_stack.pop_back();
        return;
      }

      ret.locals[instr->name] = scope_info{ instr.data, scope_stack, block };

      walk_references(instr, [&](identifier const &ref) {
        auto &local{ ret.locals.at(ref) };
        if(!scope_stack.empty()
           && (local.lowest_used_scope_stack.empty()
               || is_scope_stack_less_nested(local.definition_scope_stack,
                                             scope_stack,
                                             local.lowest_used_scope_stack)))
        {
          //util::println("new lowest scope for local {}: {}", ref, top_scope(scope_stack));
          local.lowest_used_scope_stack = scope_stack;
          local.lowest_used_block = ret.scope_to_block.at(top_scope(local.lowest_used_scope_stack));
        }
        else
        {
          //util::println("{} already has a scope just as low as {} ({})",
          //              ref,
          //              top_scope(scope_stack),
          //              top_scope(local.lowest_used_scope_stack));
        }
      });
    });

    return ret;
  }

  void hoist_locals(function &fn)
  {
    auto const collection{ collect_locals(fn) };

    for(auto const &[local, info] : collection.locals)
    {
      if(info.lowest_used_scope_stack.empty())
      {
        //if(info.inst->kind == instruction_kind::local)
        //{
        //  util::println("removing unused local {}", local);
        //  replace_with_nop(fn, info.definition_block, local);
        //}
        continue;
      }

      if(is_scope_stack_less_nested_or_equal(info.definition_scope_stack,
                                             info.definition_scope_stack,
                                             info.lowest_used_scope_stack))
      {
        //util::println("not changing {}, since it's already in a good scope (lowest: {}, def: {})",
        //              local,
        //              top_scope(info.lowest_used_scope_stack),
        //              top_scope(info.definition_scope_stack));
        continue;
      }

      if(info.inst->kind == instruction_kind::local)
      {
        //util::println("nopping local {} and moving up to scope {}",
        //              local,
        //              top_scope(info.lowest_used_scope_stack));
        replace_with_nop(fn, info.definition_block, local);

        auto const target_scope{ lcd_scope(info.definition_scope_stack,
                                           info.lowest_used_scope_stack) };
        auto const target_block{ collection.scope_to_block.at(target_scope) };
        auto &lowest_block{ fn.blocks[fn.find_block(target_block)] };
        auto const it{ std::ranges::find_if(lowest_block.instructions, [&](auto const i) {
          return i->name == target_scope;
        }) };
        jank_debug_assert(it + 1 != lowest_block.instructions.end());
        lowest_block.instructions.insert(it + 1, info.inst.data);
      }
      else
      {
        auto const target_scope{ lcd_scope(info.definition_scope_stack,
                                           info.lowest_used_scope_stack) };
        auto const target_block{ collection.scope_to_block.at(target_scope) };
        {
          auto &lowest_block{ fn.blocks[fn.find_block(target_block)] };
          auto const lowest_block_it{ std::ranges::find_if(
            lowest_block.instructions,
            [&](auto const i) { return i->name == target_scope; }) };
          jank_debug_assert(lowest_block_it + 1 != lowest_block.instructions.end());
          lowest_block.instructions.insert(
            lowest_block_it + 1,
            jtl::make_ref<inst::local>(info.inst->name,
                                       info.inst->location,
                                       analyze::cpp_util::mutable_type(info.inst->type)));
        }

        /* TODO: Better name. */
        auto const def_name{ runtime::munge(runtime::__rt_ctx->unique_string()) };

        {
          auto &def_block{ fn.blocks[fn.find_block(info.definition_block)] };
          auto const def_block_it{ std::ranges::find_if(def_block.instructions, [&](auto const i) {
            return i == info.inst.data;
          }) };
          jank_debug_assert(def_block_it + 1 != def_block.instructions.end());

          /* TODO: Better name. */
          auto const set_name{ runtime::munge(runtime::__rt_ctx->unique_string()) };
          def_block.instructions.insert(def_block_it + 1,
                                        jtl::make_ref<inst::set_local>(set_name,
                                                                       info.inst->location,
                                                                       info.inst->name,
                                                                       def_name));

          //util::println("adding local for {} in scope {}, with a set in scope {}; original def "
          //              "gets name {} and set is named {}",
          //              local,
          //              target_scope,
          //              top_scope(info.definition_scope_stack),
          //              def_name,
          //              set_name);
        }

        info.inst->name = def_name;
      }
    }
  }
}
