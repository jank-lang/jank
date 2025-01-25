#include <exception>

#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/TargetParser/Host.h>

#include <fmt/compile.h>
#include <regex>

#include <jank/native_persistent_string/fmt.hpp>
#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/core/munge.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/evaluate.hpp>
#include <jank/jit/processor.hpp>
#include <jank/util/mapped_file.hpp>
#include <jank/util/process_location.hpp>
#include <jank/util/clang_format.hpp>
#include <jank/codegen/llvm_processor.hpp>
#include <jank/profile/time.hpp>

namespace jank::runtime
{
  /* NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) */
  thread_local decltype(context::thread_binding_frames) context::thread_binding_frames{};

  /* NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) */
  context *__rt_ctx{};

  context::context()
    : context(util::cli::options{})
  {
  }

  context::context(util::cli::options const &opts)
    : jit_prc{ opts }
    , output_dir{ opts.compilation_path }
    , module_loader{ *this, opts.module_path }
  {
    auto const core(intern_ns(make_box<obj::symbol>("clojure.core")));
    auto const ns_sym(make_box<obj::symbol>("clojure.core/*ns*"));
    current_ns_var = core->intern_var(ns_sym);
    current_ns_var->bind_root(core);
    current_ns_var->dynamic.store(true);

    auto const compile_files_sym(make_box<obj::symbol>("clojure.core/*compile-files*"));
    compile_files_var = core->intern_var(compile_files_sym);
    compile_files_var->bind_root(obj::boolean::false_const());
    compile_files_var->dynamic.store(true);

    auto const assert_sym(make_box<obj::symbol>("clojure.core/*assert*"));
    assert_var = core->intern_var(assert_sym);
    assert_var->bind_root(obj::boolean::true_const());
    assert_var->dynamic.store(true);

    /* These are not actually interned. */
    current_module_var
      = make_box<runtime::var>(core, make_box<obj::symbol>("*current-module*"))->set_dynamic(true);
    no_recur_var
      = make_box<runtime::var>(core, make_box<obj::symbol>("*no-recur*"))->set_dynamic(true);
    gensym_env_var
      = make_box<runtime::var>(core, make_box<obj::symbol>("*gensym-env*"))->set_dynamic(true);

    /* TODO: Remove this once native/raw is entirely gone. */
    intern_ns(make_box<obj::symbol>("native"));

    /* This won't be set until clojure.core is loaded. */
    auto const in_ns_sym(make_box<obj::symbol>("clojure.core/in-ns"));
    in_ns_var = intern_var(in_ns_sym).expect_ok();

    push_thread_bindings(obj::persistent_hash_map::create_unique(
                           std::make_pair(current_ns_var, current_ns_var->deref())))
      .expect_ok();
  }

  context::~context()
  {
    thread_binding_frames.erase(this);
  }

  obj::symbol_ptr context::qualify_symbol(obj::symbol_ptr const &sym) const
  {
    obj::symbol_ptr qualified_sym{ sym };
    if(qualified_sym->ns.empty())
    {
      auto const current_ns(expect_object<ns>(current_ns_var->deref()));
      qualified_sym = make_box<obj::symbol>(current_ns->name->name, sym->name);
    }
    return qualified_sym;
  }

  option<var_ptr> context::find_var(obj::symbol_ptr const &sym)
  {
    profile::timer const timer{ "rt find_var" };
    if(!sym->ns.empty())
    {
      ns_ptr ns{};
      {
        auto const locked_namespaces(namespaces.rlock());
        auto const found(locked_namespaces->find(make_box<obj::symbol>("", sym->ns)));
        if(found == locked_namespaces->end())
        {
          return none;
        }
        ns = found->second;
      }

      return ns->find_var(make_box<obj::symbol>("", sym->name));
    }
    else
    {
      auto const current_ns(expect_object<ns>(current_ns_var->deref()));
      return current_ns->find_var(sym);
    }
  }

  option<var_ptr>
  context::find_var(native_persistent_string const &ns, native_persistent_string const &name)
  {
    return find_var(make_box<obj::symbol>(ns, name));
  }

  option<object_ptr> context::find_local(obj::symbol_ptr const &)
  {
    return none;
  }

  object_ptr context::eval_file(native_persistent_string_view const &path)
  {
    auto const file(util::map_file({ path.data(), path.size() }));
    if(file.is_err())
    {
      throw std::runtime_error{
        fmt::format("unable to map file {} due to error: {}", path, file.expect_err())
      };
    }
    return eval_string({ file.expect_ok().head, file.expect_ok().size });
  }

  object_ptr context::eval_string(native_persistent_string_view const &code)
  {
    profile::timer const timer{ "rt eval_string" };
    read::lex::processor l_prc{ code };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

    object_ptr ret{ obj::nil::nil_const() };
    native_vector<analyze::expression_ptr> exprs{};
    for(auto const &form : p_prc)
    {
      auto const expr(
        an_prc.analyze(form.expect_ok().unwrap().ptr, analyze::expression_position::statement));
      ret = evaluate::eval(expr.expect_ok());
      exprs.emplace_back(expr.expect_ok());
    }

    if(truthy(compile_files_var->deref()))
    {
      auto const &module(
        expect_object<runtime::ns>(intern_var("clojure.core", "*ns*").expect_ok()->deref())
          ->to_string());
      /* No matter what's in the fn, we'll return nil. */
      exprs.emplace_back(
        make_box<analyze::expression>(analyze::expr::primitive_literal<analyze::expression>{
          analyze::expression_base{ {},
                                   analyze::expression_position::tail,
                                   an_prc.root_frame,
                                   true },
          obj::nil::nil_const()
      }));
      /* TODO: Pass in module_to_load_function result */
      auto wrapped_exprs(evaluate::wrap_expressions(exprs, an_prc, module));
      auto &fn(boost::get<analyze::expr::function<analyze::expression>>(wrapped_exprs->data));
      fn.name = module::module_to_load_function(module);
      fn.unique_name = fn.name;
      codegen::llvm_processor cg_prc{ wrapped_exprs, module, codegen::compilation_target::module };
      cg_prc.gen().expect_ok();
      write_module(std::move(cg_prc.ctx)).expect_ok();
    }

    assert(ret);
    return ret;
  }

  object_ptr context::read_string(native_persistent_string_view const &code)
  {
    profile::timer const timer{ "rt read_string" };
    read::lex::processor l_prc{ code };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

    object_ptr ret{ obj::nil::nil_const() };
    for(auto const &form : p_prc)
    {
      ret = form.expect_ok().unwrap().ptr;
    }

    return ret;
  }

  native_vector<analyze::expression_ptr>
  context::analyze_string(native_persistent_string_view const &code, native_bool const eval)
  {
    profile::timer const timer{ "rt analyze_string" };
    read::lex::processor l_prc{ code };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

    native_vector<analyze::expression_ptr> ret{};
    for(auto const &form : p_prc)
    {
      auto const expr(
        an_prc.analyze(form.expect_ok().unwrap().ptr, analyze::expression_position::statement));
      if(eval)
      {
        evaluate::eval(expr.expect_ok());
      }
      ret.emplace_back(expr.expect_ok());
    }

    return ret;
  }

  result<void, native_persistent_string>
  context::load_module(native_persistent_string_view const &module, module::origin const ori)
  {
    auto const ns(current_ns());

    native_persistent_string absolute_module;
    if(module.starts_with('/'))
    {
      absolute_module = module.substr(1);
    }
    else
    {
      absolute_module = module::nest_module(ns->to_string(), module);
    }

    binding_scope const preserve{ *this };

    try
    {
      return module_loader.load(absolute_module, ori);
    }
    catch(std::exception const &e)
    {
      return err(e.what());
    }
    catch(object_ptr const &e)
    {
      return err(runtime::to_string(e));
    }
  }

  result<void, native_persistent_string>
  context::compile_module(native_persistent_string_view const &module)
  {
    module_dependencies.clear();

    binding_scope const preserve{ *this,
                                  obj::persistent_hash_map::create_unique(
                                    std::make_pair(compile_files_var, obj::boolean::true_const()),
                                    std::make_pair(current_module_var, make_box(module))) };

    return load_module(fmt::format("/{}", module), module::origin::latest);
  }

  object_ptr context::eval(object_ptr const o)
  {
    auto const expr(an_prc.analyze(o, analyze::expression_position::value));
    return evaluate::eval(expr.expect_ok());
  }

  string_result<void>
  context::write_module(std::unique_ptr<codegen::reusable_context> const codegen_ctx) const
  {
    profile::timer const timer{ fmt::format("write_module {}", codegen_ctx->module_name) };
    boost::filesystem::path const module_path{
      fmt::format("{}/{}.o", output_dir, module::module_to_resource(codegen_ctx->module_name))
    };
    boost::filesystem::create_directories(module_path.parent_path());

    /* TODO: Is there a better place for this block of code? */
    std::error_code file_error{};
    llvm::raw_fd_ostream os(module_path.c_str(), file_error, llvm::sys::fs::OpenFlags::OF_None);
    if(file_error)
    {
      return err(fmt::format("failed to open module file {} with error {}",
                             module_path.c_str(),
                             file_error.message()));
    }
    // codegen_ctx->module->print(llvm::outs(), nullptr);

    auto const target_triple{ llvm::sys::getDefaultTargetTriple() };
    std::string target_error;
    auto const target{ llvm::TargetRegistry::lookupTarget(target_triple, target_error) };
    if(!target)
    {
      return err(target_error);
    }
    llvm::TargetOptions const opt;
    auto const target_machine{
      target->createTargetMachine(target_triple, "generic", "", opt, llvm::Reloc::PIC_)
    };
    if(!target_machine)
    {
      return err(fmt::format("failed to create target machine for {}", target_triple));
    }
    llvm::legacy::PassManager pass;

    if(target_machine->addPassesToEmitFile(pass, os, nullptr, llvm::CodeGenFileType::ObjectFile))
    {
      return err(fmt::format("failed to write module to object file for {}", target_triple));
    }

    pass.run(*codegen_ctx->module);

    return ok();
  }

  native_persistent_string context::unique_string()
  {
    return unique_string("G_");
  }

  native_persistent_string context::unique_string(native_persistent_string_view const &prefix)
  {
    static native_persistent_string const dot{ "\\." };
    auto const ns{ current_ns() };
    return fmt::format(FMT_COMPILE("{}-{}-{}"),
                       runtime::munge_extra(ns->name->get_name(), dot, "_"),
                       prefix.data(),
                       ++ns->symbol_counter);
  }

  obj::symbol context::unique_symbol()
  {
    return unique_symbol("G-");
  }

  obj::symbol context::unique_symbol(native_persistent_string_view const &prefix)
  {
    return { "", unique_string(prefix) };
  }

  void context::dump() const
  {
    std::cout << "context dump\n";
    auto locked_namespaces(namespaces.rlock());
    for(auto const &p : *locked_namespaces)
    {
      std::cout << "  " << p.second->name->to_string() << "\n";
      auto locked_vars(p.second->vars.rlock());
      for(auto const &vp : (*locked_vars)->data)
      {
        auto const v(expect_object<var>(vp.second));
        if(v->deref() == nullptr)
        {
          std::cout << "    " << v->to_string() << " = nil\n";
        }
        else
        {
          std::cout << "    " << v->to_string() << " = " << runtime::to_string(v->deref()) << "\n";
        }
      }
    }
  }

  ns_ptr context::intern_ns(native_persistent_string_view const &name)
  {
    return intern_ns(make_box<obj::symbol>(name));
  }

  ns_ptr context::intern_ns(obj::symbol_ptr const &sym)
  {
    auto locked_namespaces(namespaces.wlock());
    auto const found(locked_namespaces->find(sym));
    if(found != locked_namespaces->end())
    {
      return found->second;
    }

    auto const result(locked_namespaces->emplace(sym, make_box<ns>(sym, *this)));
    return result.first->second;
  }

  option<ns_ptr> context::remove_ns(obj::symbol_ptr const &sym)
  {
    auto locked_namespaces(namespaces.wlock());
    auto const found(locked_namespaces->find(sym));
    if(found != locked_namespaces->end())
    {
      auto const ret(found->second);
      locked_namespaces->erase(found);
      return ret;
    }
    return none;
  }

  option<ns_ptr> context::find_ns(obj::symbol_ptr const &sym)
  {
    auto locked_namespaces(namespaces.rlock());
    auto const found(locked_namespaces->find(sym));
    if(found != locked_namespaces->end())
    {
      return found->second;
    }
    return none;
  }

  option<ns_ptr> context::resolve_ns(obj::symbol_ptr const &target)
  {
    auto const ns(current_ns());
    auto const alias(ns->find_alias(target));
    if(alias.is_some())
    {
      return alias.unwrap();
    }

    return find_ns(target);
  }

  /* TODO: Cache this var. */
  ns_ptr context::current_ns()
  {
    return expect_object<ns>(find_var("clojure.core", "*ns*").unwrap()->deref());
  }

  result<var_ptr, native_persistent_string>
  context::intern_var(native_persistent_string const &ns, native_persistent_string const &name)
  {
    return intern_var(make_box<obj::symbol>(ns, name));
  }

  result<var_ptr, native_persistent_string>
  context::intern_var(obj::symbol_ptr const &qualified_sym)
  {
    profile::timer const timer{ "intern_var" };
    if(qualified_sym->ns.empty())
    {
      return err(
        fmt::format("can't intern var; sym isn't qualified: {}", qualified_sym->to_string()));
    }

    auto locked_namespaces(namespaces.wlock());
    auto const found_ns(locked_namespaces->find(make_box<obj::symbol>(qualified_sym->ns)));
    if(found_ns == locked_namespaces->end())
    {
      return err(fmt::format("can't intern var; namespace doesn't exist: {}", qualified_sym->ns));
    }

    return ok(found_ns->second->intern_var(qualified_sym));
  }

  result<obj::keyword_ptr, native_persistent_string>
  context::intern_keyword(native_persistent_string_view const &ns,
                          native_persistent_string_view const &name,
                          bool const resolved)
  {
    native_persistent_string resolved_ns{ ns };
    if(!resolved)
    {
      /* The ns will be an ns alias. */
      if(!ns.empty())
      {
        auto const resolved(resolve_ns(make_box<obj::symbol>(ns)));
        if(resolved.is_none())
        {
          return err(fmt::format("Unable to resolve ns for keyword: {}", ns));
        }
        resolved_ns = resolved.unwrap()->name->name;
      }
      else
      {
        auto const current_ns(expect_object<jank::runtime::ns>(current_ns_var->deref()));
        resolved_ns = current_ns->name->name;
      }
    }
    return intern_keyword(resolved_ns.empty() ? name : fmt::format("{}/{}", resolved_ns, name));
  }

  result<obj::keyword_ptr, native_persistent_string>
  context::intern_keyword(native_persistent_string_view const &s)
  {
    profile::timer const timer{ "rt intern_keyword" };

    auto locked_keywords(keywords.wlock());
    auto const found(locked_keywords->find(s));
    if(found != locked_keywords->end())
    {
      return found->second;
    }

    auto const res(
      locked_keywords->emplace(s, make_box<obj::keyword>(detail::must_be_interned{}, s)));
    return res.first->second;
  }

  object_ptr context::macroexpand1(object_ptr const o)
  {
    profile::timer const timer{ "rt macroexpand1" };
    return visit_seqable(
      [this](auto const typed_o) -> object_ptr {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(!behavior::sequenceable<T>)
        {
          return typed_o;
        }
        else
        {
          auto const first_sym_obj(dyn_cast<obj::symbol>(first(typed_o)));
          if(!first_sym_obj)
          {
            return typed_o;
          }

          auto const var(find_var(first_sym_obj));
          /* None means it's not a var, so not a macro. No meta means no :macro set. */
          if(var.is_none() || var.unwrap()->meta.is_none())
          {
            return typed_o;
          }

          auto const meta(var.unwrap()->meta.unwrap());
          auto const found_macro(get(meta, intern_keyword("", "macro", true).expect_ok()));
          if(!found_macro || !truthy(found_macro))
          {
            return typed_o;
          }

          /* TODO: Provide &env. */
          auto const args(cons(cons(rest(typed_o), obj::nil::nil_const()), typed_o));
          return apply_to(var.unwrap()->deref(), args);
        }
      },
      [=]() { return o; },
      o);
  }

  object_ptr context::macroexpand(object_ptr const o)
  {
    auto const expanded(macroexpand1(o));
    if(expanded != o)
    {
      return macroexpand(expanded);
    }
    return o;
  }

  context::binding_scope::binding_scope(context &rt_ctx)
    : rt_ctx{ rt_ctx }
  {
    rt_ctx.push_thread_bindings().expect_ok();
  }

  context::binding_scope::binding_scope(context &rt_ctx,
                                        obj::persistent_hash_map_ptr const bindings)
    : rt_ctx{ rt_ctx }
  {
    rt_ctx.push_thread_bindings(bindings).expect_ok();
  }

  context::binding_scope::~binding_scope()
  {
    try
    {
      rt_ctx.pop_thread_bindings().expect_ok();
    }
    catch(...)
    {
      /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg): I need to log without exceptions. */
      std::printf("Exception caught while destructing binding_scope");
    }
  }

  string_result<void> context::push_thread_bindings()
  {
    auto bindings(obj::persistent_hash_map::empty());
    auto &tbfs(thread_binding_frames[this]);
    if(!tbfs.empty())
    {
      bindings = tbfs.front().bindings;
    }
    /* Nothing to preserve, if there are no current bindings. */
    else
    {
      return ok();
    }

    assert(bindings);
    return push_thread_bindings(bindings);
  }

  string_result<void> context::push_thread_bindings(object_ptr const bindings)
  {
    assert(bindings);
    if(bindings->type != object_type::persistent_hash_map)
    {
      return err(fmt::format("invalid thread binding map (must be hash map): {}",
                             runtime::to_string(bindings)));
    }

    return push_thread_bindings(expect_object<obj::persistent_hash_map>(bindings));
  }

  string_result<void> context::push_thread_bindings(obj::persistent_hash_map_ptr const bindings)
  {
    assert(bindings);
    thread_binding_frame frame{ obj::persistent_hash_map::empty() };
    auto &tbfs(thread_binding_frames[this]);
    if(!tbfs.empty())
    {
      frame.bindings = tbfs.front().bindings;
    }

    auto const thread_id(std::this_thread::get_id());

    for(auto it(bindings->fresh_seq()); it != nullptr; it = runtime::next_in_place(it))
    {
      auto const entry(it->first());
      auto const var(expect_object<var>(entry->data[0]));
      if(!var->dynamic.load())
      {
        return err(fmt::format("Can't dynamically bind non-dynamic var: {}", var->to_string()));
      }
      /* TODO: Where is this unset? */
      var->thread_bound.store(true);

      /* The binding may already be a thread binding if we're just pushing the previous
       * bindings again to give a scratch pad for some upcoming code. */
      if(entry->data[1]->type == object_type::var_thread_binding)
      {
        frame.bindings = frame.bindings->assoc(
          var,
          make_box<var_thread_binding>(expect_object<var_thread_binding>(entry->data[1])->value,
                                       thread_id));
      }
      else
      {
        frame.bindings
          = frame.bindings->assoc(var, make_box<var_thread_binding>(entry->data[1], thread_id));
      }
    }

    assert(frame.bindings);
    tbfs.push_front(std::move(frame));
    return ok();
  }

  string_result<void> context::pop_thread_bindings()
  {
    auto &tbfs(thread_binding_frames[this]);
    if(tbfs.empty())
    {
      return err("Mismatched thread binding pop");
    }

    tbfs.pop_front();

    return ok();
  }

  obj::persistent_hash_map_ptr context::get_thread_bindings() const
  {
    auto const &tbfs(thread_binding_frames[this]);
    if(tbfs.empty())
    {
      return obj::persistent_hash_map::empty();
    }
    return tbfs.front().bindings;
  }

  option<thread_binding_frame> context::current_thread_binding_frame()
  {
    auto &tbfs(thread_binding_frames[this]);
    if(tbfs.empty())
    {
      return none;
    }
    return tbfs.front();
  }
}
