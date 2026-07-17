#include <jank/runtime/context.hpp>
#include <jank/runtime/core/munge.hpp>
#include <jank/analyze/cpp_util.hpp>
#include <jank/ir/processor.hpp>
#include <jank/ir/rewrite.hpp>
#include <jank/ir/util.hpp>
#include <jank/ir/walk.hpp>
#include <jank/util/fmt/print.hpp>

/* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
#define LOG(...) (void)0;
/* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
//#define LOG(...) jank::util::println(__VA_ARGS__);

/* This IR pass walks through every block and instruction of a function, in lexical order,
 * and tracks the scope information for every instruction. It then tracks each reference
 * of each IR value and determines if any values need to be lifted up into a higher scope
 * in order to be accessible when it comes down to C++ code generation.
 *
 * This is done by keeping a scope stack for the definition of each instruction, as well
 * as the "lowest" reference to that instruction. Scope stack comparison is done by a
 * least-common-denominator algorithm, so that even sibling scope stacks will find
 * a commonality with their parent.
 *
 * The logic in this pass is built on the assumption that every function starts with
 * its own scope open/close, so that every IR value has a non-empty scope stack. */
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

  /* This is the result of our collection, which keeps track of a map of IR names to their
   * scope info and also a helpful map of scope names to blocks. */
  struct collection
  {
    native_unordered_map<identifier, scope_info> name_to_info;
    native_unordered_map<identifier, identifier> scope_to_block;
  };

  static identifier top_scope(native_deque<identifier> const &scope_stack)
  {
    return scope_stack.back();
  }

  /* Given two scope stacks, this finds the least-common-denominator. For example, given these
   * two scope stacks:
   *
   * LHS: s0 s1
   * RHS: s0 s2 s3
   *
   * The common demoninator would be s0. */
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

  /* This determines if a scope stack LHS has a lower least-common-denominator than RHS, relative
   * to the definition scope stack of an IR value. */
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
        LOG("entering scope {}", i.name);
        return;
      }
      else if(instr->kind == instruction_kind::cpp_scope_close)
      {
        LOG("exiting scope {}", scope_stack.back());
        scope_stack.pop_back();
        return;
      }

      ret.name_to_info[instr->name] = scope_info{ instr.data, scope_stack, block };

      walk_references(instr, [&](identifier const &ref) {
        /* This is a special identifier used for deferred captures. Ignore it. */
        if(ref == ":defer")
        {
          return;
        }

        auto &local{ ret.name_to_info.at(ref) };
        if(!scope_stack.empty()
           && (local.lowest_used_scope_stack.empty()
               || is_scope_stack_less_nested(local.definition_scope_stack,
                                             scope_stack,
                                             local.lowest_used_scope_stack)))
        {
          LOG("new lowest scope for local {}: {}", ref, top_scope(scope_stack));
          local.lowest_used_scope_stack = scope_stack;
          local.lowest_used_block = ret.scope_to_block.at(top_scope(local.lowest_used_scope_stack));
        }
        else
        {
          LOG("{} already has a scope just as low as {} ({})",
              ref,
              top_scope(scope_stack),
              top_scope(local.lowest_used_scope_stack));
        }
      });
    });

    return ret;
  }

  void hoist_locals(function &fn)
  {
    auto const collection{ collect_locals(fn) };

    for(auto const &[local, info] : collection.name_to_info)
    {
      /* TODO: We could nop this out, if we also nop out all of the corresponding set-locals. */
      if(info.lowest_used_scope_stack.empty())
      {
        continue;
      }

      if(is_scope_stack_less_nested_or_equal(info.definition_scope_stack,
                                             info.definition_scope_stack,
                                             info.lowest_used_scope_stack))
      {
        LOG("not changing {}, since it's already in a good scope (lowest: {}, def: {})",
            local,
            top_scope(info.lowest_used_scope_stack),
            top_scope(info.definition_scope_stack));
        continue;
      }

      /* For locals which need to be lifted, we insert a new local instruction at the new
       * scope and then we nop out the old local. */
      if(info.inst->kind == instruction_kind::local)
      {
        LOG("nopping local {} and moving up to scope {}",
            local,
            top_scope(info.lowest_used_scope_stack));
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
      /* For every other type of instruction, we insert a new local, at the LCD scope, and
       * then we insert a new set-local instruction at the previous definition scope.
       *
       * The new local has the old definition name, so we don't need to rewrite all of the
       * names. We just replace the old definition name with a placeholder, which is then
       * only used with the set-local. */
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
                                       analyze::cpp_util::non_void_type(info.inst->type)));
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

          LOG("adding local for {} in scope {}, with a set in scope {}; original def "
              "gets name {} and set is named {}",
              local,
              target_scope,
              top_scope(info.definition_scope_stack),
              def_name,
              set_name);
        }

        info.inst->name = def_name;
      }
    }
  }
}
