#include <exception>

#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/TargetParser/Host.h>

#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/core/munge.hpp>
#include <jank/runtime/core/meta.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/analyze/expr/primitive_literal.hpp>
#include <jank/evaluate.hpp>
#include <jank/jit/processor.hpp>
#include <jank/util/process_location.hpp>
#include <jank/util/clang_format.hpp>
#include <jank/util/dir.hpp>
#include <jank/util/fmt/print.hpp>
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
    , binary_cache_dir{ util::binary_cache_dir(opts.optimization_level,
                                               opts.include_dirs,
                                               opts.define_macros) }
    , module_loader{ *this, opts.module_path }
  {
    auto const core(intern_ns(make_box<obj::symbol>("clojure.core")));

    auto const file_sym(make_box<obj::symbol>("*file*"));
    current_file_var = core->intern_var(file_sym);
    current_file_var->bind_root(make_box(read::no_source_path));
    current_file_var->dynamic.store(true);

    auto const ns_sym(make_box<obj::symbol>("*ns*"));
    current_ns_var = core->intern_var(ns_sym);
    current_ns_var->bind_root(core);
    current_ns_var->dynamic.store(true);

    auto const compile_files_sym(make_box<obj::symbol>("*compile-files*"));
    compile_files_var = core->intern_var(compile_files_sym);
    compile_files_var->bind_root(obj::boolean::false_const());
    compile_files_var->dynamic.store(true);

    auto const loaded_libs_sym(make_box<obj::symbol>("*loaded-libs*"));
    loaded_libs_var = core->intern_var(loaded_libs_sym);
    loaded_libs_var->bind_root(make_box<obj::atom>(obj::persistent_sorted_set::empty()));
    loaded_libs_var->dynamic.store(true);

    auto const assert_sym(make_box<obj::symbol>("*assert*"));
    assert_var = core->intern_var(assert_sym);
    assert_var->bind_root(obj::boolean::true_const());
    assert_var->dynamic.store(true);

    /* These are not actually interned. They're extra private. */
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

  obj::symbol_ref context::qualify_symbol(obj::symbol_ref const &sym) const
  {
    obj::symbol_ref qualified_sym{ sym };
    if(qualified_sym->ns.empty())
    {
      auto const current_ns(expect_object<ns>(current_ns_var->deref()));
      qualified_sym = make_box<obj::symbol>(current_ns->name->name, sym->name);
    }
    return qualified_sym;
  }

  var_ref context::find_var(obj::symbol_ref const &sym)
  {
    profile::timer const timer{ "rt find_var" };
    if(!sym->ns.empty())
    {
      ns_ref ns{};
      {
        auto const locked_namespaces(namespaces.rlock());
        auto const found(locked_namespaces->find(make_box<obj::symbol>("", sym->ns)));
        if(found == locked_namespaces->end())
        {
          return {};
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

  var_ref context::find_var(jtl::immutable_string const &ns, jtl::immutable_string const &name)
  {
    return find_var(make_box<obj::symbol>(ns, name));
  }

  jtl::option<object_ref> context::find_local(obj::symbol_ref const &)
  {
    return none;
  }

  object_ref context::eval_file(jtl::immutable_string const &path)
  {
    auto const file(module::loader::read_file(path));
    if(file.is_err())
    {
      throw std::runtime_error{
        util::format("unable to map file {} due to error: {}", path, file.expect_err())
      };
    }

    binding_scope const preserve{ *this,
                                  obj::persistent_hash_map::create_unique(
                                    std::make_pair(current_file_var, make_box(path))) };

    return eval_string(file.expect_ok().view());
  }

  object_ref context::eval_string(native_persistent_string_view const &code)
  {
    profile::timer const timer{ "rt eval_string" };
    read::lex::processor l_prc{ code };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

    object_ref ret{ obj::nil::nil_const() };
    native_vector<analyze::expression_ref> exprs{};
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
        make_ref<analyze::expr::primitive_literal>(analyze::expression_position::tail,
                                                   an_prc.root_frame,
                                                   true,
                                                   obj::nil::nil_const()));
      /* TODO: Pass in module_to_load_function result */
      auto wrapped_exprs(evaluate::wrap_expressions(exprs, an_prc, module));
      auto fn(static_ref_cast<analyze::expr::function>(wrapped_exprs));
      fn->name = module::module_to_load_function(module);
      fn->unique_name = fn->name;
      codegen::llvm_processor cg_prc{ wrapped_exprs, module, codegen::compilation_target::module };
      cg_prc.gen().expect_ok();
      write_module(cg_prc.ctx->module_name, cg_prc.ctx->module).expect_ok();
    }

    return ret;
  }

  void context::eval_cpp_string(native_persistent_string_view const &code) const
  {
    profile::timer const timer{ "rt eval_cpp_string" };

    /* TODO: Handle all the errors here to avoid exceptions. Also, return a message that
     * is valuable to the user. */
    auto &partial_tu{ jit_prc.interpreter->Parse({ code.data(), code.size() }).get() };

    /* Writing the module before executing it because `llvm::Interpreter::Execute`
     * moves the `llvm::Module` held in the `PartialTranslationUnit`. */
    if(truthy(compile_files_var->deref()))
    {
      auto module_name{ runtime::to_string(current_module_var->deref()) };
      write_module(module_name, partial_tu.TheModule).expect_ok();
    }

    auto err(jit_prc.interpreter->Execute(partial_tu));
  }

  object_ref context::read_string(native_persistent_string_view const &code)
  {
    profile::timer const timer{ "rt read_string" };

    /* When reading an arbitrary string, we don't want the last *current-file* to
     * be set as source file, so we need to bind it to nil. */
    binding_scope const preserve{ *this,
                                  obj::persistent_hash_map::create_unique(
                                    std::make_pair(current_file_var, obj::nil::nil_const())) };

    read::lex::processor l_prc{ code };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

    object_ref ret{ obj::nil::nil_const() };
    for(auto const &form : p_prc)
    {
      ret = form.expect_ok().unwrap().ptr;
    }

    return ret;
  }

  native_vector<analyze::expression_ref>
  context::analyze_string(native_persistent_string_view const &code, native_bool const eval)
  {
    profile::timer const timer{ "rt analyze_string" };
    read::lex::processor l_prc{ code };
    read::parse::processor p_prc{ l_prc.begin(), l_prc.end() };

    native_vector<analyze::expression_ref> ret{};
    for(auto const &form : p_prc)
    {
      auto const expr(
        an_prc.analyze(form.expect_ok().unwrap().ptr, analyze::expression_position::statement));
      if(eval)
      {
        if(expr.is_err())
        {
          util::println("{}", expr.expect_err()->message);
        }
        evaluate::eval(expr.expect_ok());
      }
      ret.emplace_back(expr.expect_ok());
    }

    return ret;
  }

  jtl::result<void, jtl::immutable_string>
  context::load_module(native_persistent_string_view const &module, module::origin const ori)
  {
    auto const ns(current_ns());

    jtl::immutable_string absolute_module;
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
    catch(object_ref const &e)
    {
      return err(runtime::to_string(e));
    }
  }

  jtl::result<void, jtl::immutable_string>
  context::compile_module(native_persistent_string_view const &module)
  {
    module_dependencies.clear();

    binding_scope const preserve{ *this,
                                  obj::persistent_hash_map::create_unique(
                                    std::make_pair(compile_files_var, obj::boolean::true_const()),
                                    std::make_pair(current_module_var, make_box(module))) };

    return load_module(util::format("/{}", module), module::origin::latest);
  }

  object_ref context::eval(object_ref const o)
  {
    auto const expr(an_prc.analyze(o, analyze::expression_position::value));
    return evaluate::eval(expr.expect_ok());
  }

  jtl::string_result<void> context::write_module(jtl::immutable_string const &module_name,
                                                 std::unique_ptr<llvm::Module> const &module) const
  {
    profile::timer const timer{ util::format("write_module {}", module_name) };
    std::filesystem::path const module_path{
      util::format("{}/{}.o", binary_cache_dir, module::module_to_path(module_name))
    };
    std::filesystem::create_directories(module_path.parent_path());

    /* TODO: Is there a better place for this block of code? */
    std::error_code file_error{};
    llvm::raw_fd_ostream os(module_path.c_str(), file_error, llvm::sys::fs::OpenFlags::OF_None);
    if(file_error)
    {
      return err(util::format("failed to open module file {} with error {}",
                              module_path.c_str(),
                              file_error.message()));
    }
    //codegen_ctx->module->print(llvm::outs(), nullptr);

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
      return err(util::format("failed to create target machine for {}", target_triple));
    }
    llvm::legacy::PassManager pass;

    if(target_machine->addPassesToEmitFile(pass, os, nullptr, llvm::CodeGenFileType::ObjectFile))
    {
      return err(util::format("failed to write module to object file for {}", target_triple));
    }

    pass.run(*module);

    return ok();
  }

  jtl::immutable_string context::unique_string() const
  {
    return unique_string("G_");
  }

  jtl::immutable_string context::unique_string(native_persistent_string_view const &prefix) const
  {
    static jtl::immutable_string const dot{ "\\." };
    auto const ns{ current_ns() };
    return util::format("{}-{}-{}",
                        runtime::munge_extra(ns->name->get_name(), dot, "_"),
                        prefix.data(),
                        ++ns->symbol_counter);
  }

  obj::symbol context::unique_symbol() const
  {
    return unique_symbol("G-");
  }

  obj::symbol context::unique_symbol(native_persistent_string_view const &prefix) const
  {
    return { "", unique_string(prefix) };
  }

  ns_ref context::intern_ns(jtl::immutable_string const &name)
  {
    return intern_ns(make_box<obj::symbol>(name));
  }

  ns_ref context::intern_ns(obj::symbol_ref const &sym)
  {
    if(!sym->ns.empty())
    {
      throw std::runtime_error{ util::format("Can't intern ns. Sym is qualified: {}",
                                             sym->to_string()) };
    }
    auto locked_namespaces(namespaces.wlock());
    auto const found(locked_namespaces->find(sym));
    if(found != locked_namespaces->end())
    {
      return found->second;
    }

    auto const result(locked_namespaces->emplace(sym, make_box<ns>(sym, *this)));
    return result.first->second;
  }

  ns_ref context::remove_ns(obj::symbol_ref const &sym)
  {
    auto locked_namespaces(namespaces.wlock());
    auto const found(locked_namespaces->find(sym));
    if(found != locked_namespaces->end())
    {
      auto const ret(found->second);
      locked_namespaces->erase(found);
      return ret;
    }
    return {};
  }

  ns_ref context::find_ns(obj::symbol_ref const &sym)
  {
    auto locked_namespaces(namespaces.rlock());
    auto const found(locked_namespaces->find(sym));
    if(found != locked_namespaces->end())
    {
      return found->second;
    }
    return {};
  }

  ns_ref context::resolve_ns(obj::symbol_ref const &target)
  {
    auto const ns(current_ns());
    auto const alias(ns->find_alias(target));
    if(alias.is_some())
    {
      return alias;
    }

    return find_ns(target);
  }

  ns_ref context::current_ns() const
  {
    return expect_object<ns>(current_ns_var->deref());
  }

  jtl::result<var_ref, jtl::immutable_string>
  context::intern_var(jtl::immutable_string const &ns, jtl::immutable_string const &name)
  {
    return intern_var(make_box<obj::symbol>(ns, name));
  }

  jtl::result<var_ref, jtl::immutable_string>
  context::intern_var(obj::symbol_ref const &qualified_sym)
  {
    profile::timer const timer{ "intern_var" };
    if(qualified_sym->ns.empty())
    {
      return err(
        util::format("Can't intern var. Sym isn't qualified: {}", qualified_sym->to_string()));
    }

    auto locked_namespaces(namespaces.wlock());
    auto const found_ns(locked_namespaces->find(make_box<obj::symbol>(qualified_sym->ns)));
    if(found_ns == locked_namespaces->end())
    {
      return err(util::format("Can't intern var. Namespace doesn't exist: {}", qualified_sym->ns));
    }

    return ok(found_ns->second->intern_var(qualified_sym));
  }

  jtl::result<obj::keyword_ref, jtl::immutable_string>
  context::intern_keyword(jtl::immutable_string const &ns,
                          jtl::immutable_string const &name,
                          bool const resolved)
  {
    jtl::immutable_string resolved_ns{ ns };
    if(!resolved)
    {
      /* The ns will be an ns alias. */
      if(!ns.empty())
      {
        auto const resolved(current_ns()->find_alias(make_box<obj::symbol>(ns)));
        if(resolved.is_nil())
        {
          return err(util::format("Unable to resolve namespace alias '{}'", ns));
        }
        resolved_ns = resolved->name->name;
      }
      else
      {
        auto const current_ns(expect_object<jank::runtime::ns>(current_ns_var->deref()));
        resolved_ns = current_ns->name->name;
      }
    }
    return intern_keyword(resolved_ns.empty() ? name : util::format("{}/{}", resolved_ns, name));
  }

  jtl::result<obj::keyword_ref, jtl::immutable_string>
  context::intern_keyword(jtl::immutable_string const &s)
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

  object_ref context::macroexpand1(object_ref const o)
  {
    profile::timer const timer{ "rt macroexpand1" };
    return visit_seqable(
      [this](auto const typed_o) -> object_ref {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(!behavior::sequenceable<T>)
        {
          return typed_o;
        }
        else
        {
          auto const first_sym_obj(dyn_cast<obj::symbol>(first(typed_o)));
          if(first_sym_obj.is_nil())
          {
            return typed_o;
          }

          auto const var(find_var(first_sym_obj));
          /* None means it's not a var, so not a macro. No meta means no :macro set. */
          if(var.is_nil() || var->meta.is_none())
          {
            return typed_o;
          }

          auto const meta(var->meta.unwrap());
          auto const found_macro(get(meta, intern_keyword("", "macro", true).expect_ok()));
          if(found_macro.is_nil() || !truthy(found_macro))
          {
            return typed_o;
          }

          /* TODO: Provide &env. */
          auto const args(cons(cons(rest(typed_o), obj::nil::nil_const()), typed_o));
          return apply_to(var->deref(), args);
        }
      },
      [=]() { return o; },
      o);
  }

  object_ref context::macroexpand(object_ref const o)
  {
    auto expanded(macroexpand1(o));
    if(expanded != o)
    {
      /* If we've actually expanded `o` into something else, it's helpful to update the meta
       * on the expanded data to tie it back to the original form. */
      auto const source{ object_source(o) };
      if(source != read::source::unknown)
      {
        auto meta{ runtime::meta(expanded) };
        auto const macro_kw{ __rt_ctx->intern_keyword("jank/macro-expansion").expect_ok() };
        meta = runtime::assoc(meta, macro_kw, o);
        expanded = with_meta_graceful(expanded, meta);
      }

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
                                        obj::persistent_hash_map_ref const bindings)
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
      util::println("Exception caught while destructing binding_scope");
    }
  }

  jtl::string_result<void> context::push_thread_bindings()
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

    return push_thread_bindings(bindings);
  }

  jtl::string_result<void> context::push_thread_bindings(object_ref const bindings)
  {
    if(bindings->type != object_type::persistent_hash_map)
    {
      return err(util::format("invalid thread binding map (must be hash map): {}",
                              runtime::to_string(bindings)));
    }

    return push_thread_bindings(expect_object<obj::persistent_hash_map>(bindings));
  }

  jtl::string_result<void>
  context::push_thread_bindings(obj::persistent_hash_map_ref const bindings)
  {
    thread_binding_frame frame{ obj::persistent_hash_map::empty() };
    auto &tbfs(thread_binding_frames[this]);
    if(!tbfs.empty())
    {
      frame.bindings = tbfs.front().bindings;
    }

    auto const thread_id(std::this_thread::get_id());

    for(auto it(bindings->fresh_seq()); it.is_some(); it = it->next_in_place())
    {
      auto const entry(it->first());
      auto const var(expect_object<var>(entry->data[0]));
      if(!var->dynamic.load())
      {
        return err(util::format("Can't dynamically bind non-dynamic var: {}", var->to_string()));
      }

      /* XXX: Once this is set to true, here, it's never unset. */
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

    tbfs.push_front(std::move(frame));
    return ok();
  }

  jtl::string_result<void> context::pop_thread_bindings()
  {
    auto &tbfs(thread_binding_frames[this]);
    if(tbfs.empty())
    {
      return err("Mismatched thread binding pop");
    }

    tbfs.pop_front();

    return ok();
  }

  obj::persistent_hash_map_ref context::get_thread_bindings() const
  {
    auto const &tbfs(thread_binding_frames[this]);
    if(tbfs.empty())
    {
      return obj::persistent_hash_map::empty();
    }
    return tbfs.front().bindings;
  }

  jtl::option<thread_binding_frame> context::current_thread_binding_frame()
  {
    auto &tbfs(thread_binding_frames[this]);
    if(tbfs.empty())
    {
      return none;
    }
    return tbfs.front();
  }
}
