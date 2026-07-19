#include <fstream>

#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/TargetParser/Host.h>

#include <CppInterOp/CppInterOp.h>
#include <CppInterOp/Compatibility.h>

#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/core/munge.hpp>
#include <jank/runtime/core/meta.hpp>
#include <jank/runtime/core/call.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/analyze/expr/primitive_literal.hpp>
#include <jank/analyze/pass/optimize.hpp>
#include <jank/evaluate.hpp>
#include <jank/jit/processor.hpp>
#include <jank/util/clang.hpp>
#include <jank/util/clang_format.hpp>
#include <jank/util/environment.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/util/scope_exit.hpp>
#include <jank/ir/processor.hpp>
#include <jank/codegen/cpp_processor.hpp>
#include <jank/codegen/optimize.hpp>
#include <jank/aot/processor.hpp>
#include <jank/error/codegen.hpp>
#include <jank/error/runtime.hpp>
#include <jank/profile/time.hpp>

namespace jank::runtime
{
  jtl::option<object_ref> context::eval_file(jtl::immutable_string const &path)
  {
    auto const file(module::loader::read_file(path));
    if(file.is_err())
    {
      throw file.expect_err();
    }

    binding_scope const preserve{ obj::persistent_hash_map::create_unique(
      std::make_pair(current_file_var, make_box(path))) };

    return eval_string(file.expect_ok().view());
  }

  jtl::option<object_ref>
  context::eval_string(jtl::immutable_string const &code, read::source_position const &p) const
  {
    profile::timer const timer{ "rt eval_string" };
    read::lex::processor l_prc{ code, p };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

    bool no_op{ true };
    object_ref ret{};
    native_vector<object_ref> forms{};
    for(auto const &form : p_prc)
    {
      if(no_op && form.expect_ok().is_none())
      {
        continue;
      }

      no_op = false;
      analyze::processor an_prc;
      auto const expr(analyze::pass::optimize(
        an_prc.analyze(form.expect_ok().unwrap().ptr, analyze::expression_position::statement)
          .expect_ok()));
      ret = evaluate::eval(expr);

      forms.emplace_back(form.expect_ok().unwrap().ptr);
    }

    if(no_op)
    {
      return jtl::none;
    }

    /* TODO: Analyze only once so macros are expanded only once. */
    /* When compiling, we analyze twice. This is because eval will modify its expression
     * in order to wrap it in a function. Undoing this is arduous and error prone, so
     * we just don't bother.
     *
     * Furthermore, module compilation may be different from JIT compilation, since it's
     * targeted at AOT and doesn't have access to what's loaded in the JIT runtime. */
    if(truthy(compile_files_var->deref()))
    {
      profile::timer const timer{ "rt compile-module" };
      auto const &module(runtime::to_string(current_module_var->deref()));
      auto const name{ module::module_to_load_function(module) };

      if(forms.empty())
      {
        forms.emplace_back(jank_nil);
      }

      auto const form{ runtime::conj(
        runtime::conj(runtime::conj(make_box<obj::native_vector_sequence>(jtl::move(forms)),
                                    obj::persistent_vector::empty()),
                      make_box<obj::symbol>(name)),
        make_box<obj::symbol>("fn*")) };
      analyze::processor an_prc;
      auto const expr(analyze::pass::optimize(
        an_prc.analyze(form, analyze::expression_position::statement).expect_ok()));
      auto const fn{ static_box_cast<analyze::expr::function>(expr) };
      fn->unique_name = name;
      auto const mod{ ir::create(fn, module, codegen::compilation_target::module) };

      auto const generated{ codegen::gen_cpp(mod) };
      auto const &code{ generated.declaration };

      jtl::immutable_string_view const print_settings{ getenv("JANK_PRINT_CODEGEN") ?: "" };
      if(print_settings == "1")
      {
        auto const formatted{ util::format_cpp_source(code).expect_ok() };
        util::println("\n{}\n", formatted);
      }

      auto const module_name{ runtime::to_string(current_module_var->deref()) };
      auto parse_res{ jit_prc.interpreter->Parse({ code.data(), code.size() }) };
      if(!parse_res)
      {
        /* TODO: Helper to turn an llvm::Error into a string. */
        jtl::immutable_string const res{ "Unable to compile generated C++ source." };
        llvm::logAllUnhandledErrors(parse_res.takeError(), llvm::errs(), "error: ");
        throw error::internal_codegen_failure(res);
      }
      auto &partial_tu{ parse_res.get() };
      if(util::cli::opts.output_target != util::cli::compilation_target::cpp)
      {
        codegen::optimize(partial_tu.TheModule.get(), module_name);
      }
      write_module(module_name, code, partial_tu.TheModule.get()).expect_ok();
    }

    return ret;
  }

  jtl::option<object_ref> context::eval_string(jtl::immutable_string const &code) const
  {
    read::source_position const p{};
    return eval_string(code, p);
  }

  jtl::result<void, error_ref> context::eval_cpp_string(jtl::immutable_string const &code) const
  {
    profile::timer const timer{ "rt eval_cpp_string" };

    auto parse_res{ jit_prc.interpreter->Parse({ code.data(), code.size() }) };
    if(!parse_res)
    {
      /* TODO: Helper to turn an llvm::Error into a string. */
      llvm::logAllUnhandledErrors(parse_res.takeError(), llvm::errs(), "error: ");
      return error::runtime_invalid_cpp_eval();
    }
    auto &partial_tu{ parse_res.get() };

    /* Writing the module before executing it because `llvm::Interpreter::Execute`
     * moves the `llvm::Module` held in the `PartialTranslationUnit`. */
    if(truthy(compile_files_var->deref()))
    {
      auto module_name{ runtime::to_string(current_module_var->deref()) };
      write_module(module_name, code, partial_tu.TheModule.get()).expect_ok();
    }

    auto exec_res(jit_prc.interpreter->Execute(partial_tu));
    if(exec_res)
    {
      llvm::logAllUnhandledErrors(std::move(exec_res), llvm::errs(), "error: ");
      return error::runtime_invalid_cpp_eval();
    }
    return ok();
  }

  native_vector<analyze::expression_ref>
  context::analyze_string(jtl::immutable_string const &code, bool const eval)
  {
    profile::timer const timer{ "rt analyze_string" };
    read::lex::processor l_prc{ code };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

    analyze::processor an_prc;
    native_vector<analyze::expression_ref> ret{};
    for(auto const &form : p_prc)
    {
      if(eval)
      {
        auto const expr(
          an_prc.analyze(form.expect_ok().unwrap().ptr, analyze::expression_position::statement));
        if(expr.is_err())
        {
          util::println("{}", expr.expect_err()->message);
        }
        evaluate::eval(analyze::pass::optimize(expr.expect_ok()));
      }

      auto const expr(analyze::pass::optimize(
        an_prc.analyze(form.expect_ok().unwrap().ptr, analyze::expression_position::statement)
          .expect_ok()));
      ret.emplace_back(expr);
    }

    return ret;
  }

  object_ref
  context::read_file(jtl::immutable_string const &file_path, object_ref const reader_opts)
  {
    auto const file(module::loader::read_file(file_path));
    if(file.is_err())
    {
      throw file.expect_err();
    }

    return read_string(file.expect_ok().data(), reader_opts, std::numeric_limits<u64>::max());
  }

  jtl::result<void, error_ref> context::compile_module(jtl::immutable_string const &module)
  {
    binding_scope const preserve{ obj::persistent_hash_map::create_unique(
      std::make_pair(compile_files_var, jank_true)) };

    return load_module(module, module::origin::latest);
  }

  object_ref context::eval(object_ref const o)
  {
    analyze::processor an_prc;
    auto const expr(
      analyze::pass::optimize(an_prc.analyze(o, analyze::expression_position::value).expect_ok()));
    return evaluate::eval(expr);
  }

  jtl::immutable_string
  context::get_output_module_name(jtl::immutable_string const &module_name) const
  {
    return util::cli::opts.output_module_filename.empty()
      ? util::format("{}/{}.{}",
                     util::build_dir(),
                     module::module_to_path(module_name),
                     util::cli::compilation_target_extension(util::cli::opts.output_target))
      : jtl::immutable_string{ util::cli::opts.output_module_filename };
  }

  jtl::string_result<void> context::write_module(jtl::immutable_string const &module_name,
                                                 jtl::immutable_string const &cpp_code,
                                                 jtl::ref<llvm::Module> const &module) const
  {
    profile::timer const timer{ util::format("write_module {}", module_name) };
    std::filesystem::path const module_path{ get_output_module_name(module_name).c_str() };
    auto const &module_dir{ module_path.parent_path() };
    if(!module_dir.empty())
    {
      std::filesystem::create_directories(module_dir);
    }

    //util::println("writing module {} to {}", module_name, module_path.c_str());

    switch(util::cli::opts.output_target)
    {
      case util::cli::compilation_target::cpp:
        {
          std::ofstream ofs{ module_path.c_str() };
          ofs << "#include <jank/prelude.hpp>\n";
          ofs << cpp_code;
          return ok();
        }
      case util::cli::compilation_target::object:
        {
          /* TODO: Is there a better place for this block of code? */
          std::error_code file_error{};
          llvm::raw_fd_ostream os(module_path.string(),
                                  file_error,
                                  llvm::sys::fs::OpenFlags::OF_None);
          if(file_error)
          {
            return err(util::format("Failed to open module file '{}' with error '{}'.",
                                    module_path.c_str(),
                                    file_error.message()));
          }
          //module->print(llvm::outs(), nullptr);

          auto const target_triple{ util::default_target_triple() };
          std::string target_error;
          auto const target{
            llvm::TargetRegistry::lookupTarget(llvm::Triple{ target_triple.c_str() }, target_error)
          };
          if(!target)
          {
            return err(target_error);
          }
          llvm::TargetOptions const opt;
          llvm::CodeGenOptLevel level{ llvm::CodeGenOptLevel::Default };
          switch(util::cli::opts.codegen_optimization_level)
          {
            case 0:
              level = llvm::CodeGenOptLevel::None;
              break;
            case 1:
              level = llvm::CodeGenOptLevel::Less;
              break;
            case 2:
              level = llvm::CodeGenOptLevel::Default;
              break;
            case 3:
              level = llvm::CodeGenOptLevel::Aggressive;
              break;
            default:
              break;
          }

          auto const target_machine{ target->createTargetMachine(
            llvm::Triple{ target_triple.c_str() },
            "generic",
            "",
            opt,
            llvm::Reloc::PIC_,
            llvm::CodeModel::Large,
            level) };
          if(!target_machine)
          {
            return err(util::format("Failed to create target machine for '{}'.", target_triple));
          }
          llvm::legacy::PassManager pass;

          if(target_machine->addPassesToEmitFile(pass,
                                                 os,
                                                 nullptr,
                                                 llvm::CodeGenFileType::ObjectFile))
          {
            return err(
              util::format("Failed to write module to object file for '{}'.", target_triple));
          }

          pass.run(*module);
          return ok();
        }
      case util::cli::compilation_target::unspecified:
      default:
        return err(util::format("Unable to write module, given output target '{}'.",
                                util::cli::compilation_target_str(util::cli::opts.output_target)));
    }
  }

}
