#include <vector>

#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DebugProgramInstruction.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>

#include <jank/debug/local_names.hpp>
#include <jank/runtime/core/munge.hpp>
#include <jank/util/cli.hpp>

namespace jank::debug
{
  /* Encoded names are repeated at every use in the generated C++, so the encoding keeps
   * ordinary identifier characters as they are and only escapes the rest as `Z` plus two hex
   * digits. `Z` is escaped too, which also frees `_` to separate the unique suffix. */
  static constexpr std::string_view prefix{ "_jd_" };
  static constexpr char escape{ 'Z' };

  static bool is_plain(char const c)
  {
    return c != escape
      && ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'));
  }

  static int nibble(char const c)
  {
    if(c >= '0' && c <= '9')
    {
      return c - '0';
    }
    if(c >= 'a' && c <= 'f')
    {
      return c - 'a' + 10;
    }
    return -1;
  }

  std::string encode_local(std::string_view const &raw_name, usize const unique)
  {
    static constexpr char const *digits{ "0123456789abcdef" };
    std::string ret{ prefix };
    for(auto const c : raw_name)
    {
      if(is_plain(c))
      {
        ret += c;
        continue;
      }
      auto const byte{ static_cast<unsigned char>(c) };
      ret += escape;
      ret += digits[byte >> 4];
      ret += digits[byte & 0xf];
    }
    ret += '_';
    ret += std::to_string(unique);
    return ret;
  }

  std::string parameter_name(std::string_view const &raw_name, usize const index)
  {
    if(util::cli::opts.debug)
    {
      return encode_local(raw_name, index);
    }
    return runtime::munge(jtl::immutable_string{ raw_name.data(), raw_name.size() }).c_str();
  }

  jtl::option<std::string> decode_local(std::string_view const &encoded)
  {
    if(!encoded.starts_with(prefix))
    {
      return jtl::none;
    }

    auto const rest{ encoded.substr(prefix.size()) };
    auto const sep{ rest.rfind('_') };
    if(sep == std::string_view::npos || sep + 1 == rest.size())
    {
      return jtl::none;
    }

    std::string ret;
    for(usize i{}; i < sep; ++i)
    {
      if(rest[i] != escape)
      {
        if(!is_plain(rest[i]))
        {
          return jtl::none;
        }
        ret += rest[i];
        continue;
      }

      if(i + 2 >= sep)
      {
        return jtl::none;
      }
      auto const hi{ nibble(rest[i + 1]) };
      auto const lo{ nibble(rest[i + 2]) };
      if(hi < 0 || lo < 0)
      {
        return jtl::none;
      }
      ret += static_cast<char>((hi << 4) | lo);
      i += 2;
    }
    return ret;
  }

  static bool rename_variable(llvm::DILocalVariable * const var)
  {
    if(!var)
    {
      return false;
    }

    auto const decoded{ decode_local(var->getName().str()) };
    if(decoded.is_none())
    {
      return false;
    }

    /* The name is operand 1 of a DILocalVariable. Replacing it re-uniques the node, so
     * every user of the variable sees the new name. */
    var->replaceOperandWith(1, llvm::MDString::get(var->getContext(), decoded.unwrap()));
    return true;
  }

  /* At -O0, a `declare` gives a local one location for the whole function, so a debugger
   * shows slots for locals whose declaration hasn't run yet as garbage. A `value` record which
   * dereferences the slot starts the variable's location range at its declaration instead. */
  static void restrict_to_declaration(llvm::DbgVariableRecord &record,
                                      std::vector<llvm::DbgVariableRecord *> &converted)
  {
    if(!record.isDbgDeclare() || !record.getVariable() || !record.getExpression())
    {
      return;
    }

    /* Locals are scoped to the `#line` file switches in generated code, which are lexical block
     * files. LLVM drops value-located variables in those, so they're scoped to the enclosing
     * real scope instead. */
    auto const var{ record.getVariable() };
    if(auto const file_scope{ llvm::dyn_cast_or_null<llvm::DILexicalBlockFile>(var->getScope()) })
    {
      var->replaceOperandWith(0, file_scope->getNonLexicalBlockFileScope());
    }

    record.Type = llvm::DbgVariableRecord::LocationType::Value;
    record.setExpression(
      llvm::DIExpression::prepend(record.getExpression(), llvm::DIExpression::DerefBefore));
    converted.push_back(&record);
  }

  /* Clang places a local's record before its initialization. A debugger which stops right
   * there, like at a `__builtin_debugtrap`, would show the slot before it's constructed, so
   * the record moves past the first instruction which uses the slot. */
  static void move_after_initialization(llvm::DbgVariableRecord &record)
  {
    auto const first{ record.getInstruction() };
    if(!first || record.getNumVariableLocationOps() != 1)
    {
      return;
    }

    auto const slot{ record.getVariableLocationOp(0) };
    for(auto inst{ first }; inst && !inst->isTerminator(); inst = inst->getNextNode())
    {
      if(llvm::is_contained(inst->operands(), slot))
      {
        record.removeFromParent();
        auto const next{ inst->getNextNode() };
        /* FastISel hoists a debug record which directly precedes a call, since it hands calls
         * to the slower selector. That would start the variable's range at the top of the
         * function again, so a volatile load of the slot (which FastISel keeps) separates them. */
        if(next && llvm::isa<llvm::CallBase>(next))
        {
          auto const load{ new llvm::LoadInst(llvm::Type::getInt8Ty(inst->getContext()),
                                              slot,
                                              "",
                                              true,
                                              llvm::Align{ 1 },
                                              next->getIterator()) };
          load->setDebugLoc(inst->getDebugLoc());
        }
        inst->getParent()->insertDbgRecordAfter(&record, inst);
        return;
      }
    }
  }

  static void
  collect_subprograms(llvm::DILocation const *loc, llvm::SmallPtrSetImpl<llvm::DISubprogram *> &out)
  {
    for(; loc; loc = loc->getInlinedAt())
    {
      if(auto const scope{ loc->getScope() })
      {
        if(auto const sp{ scope->getSubprogram() })
        {
          out.insert(sp);
        }
      }
    }
  }

  /* When optimizations remove a variable's last debug record, the variable lives on only in its
   * subprogram's retained nodes, which is where debuggers still find it to print
   * `<optimized out>`. Those variables must be renamed as well. */
  usize rewrite_local_names(llvm::Module &module)
  {
    usize count{};
    llvm::SmallPtrSet<llvm::DISubprogram *, 16> subprograms;
    std::vector<llvm::DbgVariableRecord *> converted;
    auto const visit_records
      = [&](llvm::iterator_range<llvm::simple_ilist<llvm::DbgRecord>::iterator> const range) {
          for(auto const record : llvm::filterDbgVars(range))
          {
            count += rename_variable(record.get().getVariable());
            restrict_to_declaration(record.get(), converted);
          }
        };

    for(auto &fn : module)
    {
      if(auto const sp{ fn.getSubprogram() })
      {
        subprograms.insert(sp);
      }
      for(auto &block : fn)
      {
        for(auto &inst : block)
        {
          visit_records(inst.getDbgRecordRange());
          if(auto const intrinsic{ llvm::dyn_cast<llvm::DbgVariableIntrinsic>(&inst) })
          {
            count += rename_variable(intrinsic->getVariable());
          }
          collect_subprograms(inst.getDebugLoc().get(), subprograms);
        }
        if(auto const trailing{ block.getTrailingDbgRecords() })
        {
          visit_records(trailing->getDbgRecordRange());
        }
      }
    }

    for(auto const record : converted)
    {
      move_after_initialization(*record);
    }

    for(auto const sp : subprograms)
    {
      std::vector<llvm::DILocalVariable *> vars;
      for(auto const node : sp->getRetainedNodes())
      {
        if(auto const var{ llvm::dyn_cast_or_null<llvm::DILocalVariable>(node) })
        {
          vars.push_back(var);
        }
      }
      for(auto const var : vars)
      {
        count += rename_variable(var);
      }
    }
    return count;
  }
}
