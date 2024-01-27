#include <exception>

#include <fmt/compile.h>

#include <jank/read/lex.hpp>
#include <jank/read/parse.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/obj/native_function_wrapper.hpp>
#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/obj/number.hpp>
#include <jank/runtime/util.hpp>
#include <jank/runtime/seq.hpp>
#include <jank/analyze/processor.hpp>
#include <jank/codegen/processor.hpp>
#include <jank/evaluate.hpp>
#include <jank/jit/processor.hpp>
#include <jank/util/mapped_file.hpp>
#include <jank/util/process_location.hpp>

namespace jank::runtime
{
  /* NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) */
  thread_local decltype(context::thread_binding_frames) context::thread_binding_frames{};

  context::context()
    : context(util::cli::options{})
  {
  }

  context::context(util::cli::options const &opts)
    : jit_prc{ *this, opts.optimization_level }
    , output_dir{ opts.compilation_path }
    , module_loader{ *this, opts.class_path }
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

    intern_ns(make_box<obj::symbol>("native"));

    /* This won't be set until clojure.core is loaded. */
    auto const in_ns_sym(make_box<obj::symbol>("clojure.core/in-ns"));
    in_ns_var = intern_var(in_ns_sym).expect_ok();

    /* TODO: Remove this once it can be defined in jank. */
    auto const seq_sym(make_box<obj::symbol>("clojure.core/seq"));
    intern_var(seq_sym).expect_ok()->bind_root(
      make_box<obj::native_function_wrapper>(static_cast<object_ptr (*)(object_ptr)>(&seq)));

    auto const fresh_seq_sym(make_box<obj::symbol>("clojure.core/fresh-seq"));
    intern_var(fresh_seq_sym)
      .expect_ok()
      ->bind_root(make_box<obj::native_function_wrapper>(&fresh_seq));

    push_thread_bindings(obj::persistent_hash_map::create_unique(
                           std::make_pair(current_ns_var, current_ns_var->deref())))
      .expect_ok();
  }

  context::context(context const &ctx)
    : jit_prc{ *this, ctx.jit_prc.optimization_level }
    , module_dependencies{ ctx.module_dependencies }
    , output_dir{ ctx.output_dir }
    , module_loader{ *this, ctx.module_loader.paths }
  {
    {
      auto ns_lock(namespaces.wlock());
      for(auto const &ns : *ctx.namespaces.rlock())
      {
        ns_lock->insert({ ns.first, ns.second->clone(*this) });
      }
      *keywords.wlock() = *ctx.keywords.rlock();
    }

    auto &tbfs(thread_binding_frames[this]);
    auto const &other_tbfs(thread_binding_frames[&ctx]);
    for(auto const &v : other_tbfs)
    {
      thread_binding_frame frame{ obj::persistent_hash_map::empty() };
      for(auto it(v.bindings->fresh_seq()); it != nullptr; it = it->next_in_place())
      {
        auto const entry(it->first());
        auto const var(expect_object<var>(entry->data[0]));
        auto const value(entry->data[1]);
        auto const new_var(intern_var(var->n->name->name, var->name->name).expect_ok());
        frame.bindings = frame.bindings->assoc(new_var, value);
      }

      /* We push to the back, since we're looping from the front of the other list. If we
       * pushed to the front of this one, we'd reverse the order. */
      tbfs.push_back(std::move(frame));
    }

    auto const core(intern_ns(make_box<obj::symbol>("clojure.core")));
    current_ns_var = core->intern_var(make_box<obj::symbol>("clojure.core/*ns*"));

    in_ns_var = intern_var(make_box<obj::symbol>("clojure.core/in-ns")).expect_ok();
    compile_files_var
      = intern_var(make_box<obj::symbol>("clojure.core/*compile-files*")).expect_ok();
    assert_var = core->intern_var(make_box<obj::symbol>("clojure.core/*assert*"));

    current_module_var
      = make_box<runtime::var>(core, make_box<obj::symbol>("*current-module*"))->set_dynamic(true);
    no_recur_var
      = make_box<runtime::var>(core, make_box<obj::symbol>("*no-recur*"))->set_dynamic(true);
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
    profile::timer timer{ "rt find_var" };
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
    auto const file(util::map_file(path));
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
    profile::timer timer{ "rt eval_string" };
    read::lex::processor l_prc{ code };
    read::parse::processor p_prc{ *this, l_prc.begin(), l_prc.end() };

    object_ptr ret{ obj::nil::nil_const() };
    native_vector<analyze::expression_ptr> exprs{};
    for(auto const &form : p_prc)
    {
      auto const expr(an_prc.analyze(form.expect_ok(), analyze::expression_type::statement));
      ret = evaluate::eval(*this, jit_prc, expr.expect_ok());
      exprs.emplace_back(expr.expect_ok());
    }

    if(detail::truthy(compile_files_var->deref()))
    {
      auto const &current_module(
        expect_object<obj::persistent_string>(current_module_var->deref())->data);
      auto wrapped_exprs(evaluate::wrap_expressions(exprs, an_prc));
      wrapped_exprs.name = "__ns";
      auto const &module(
        expect_object<runtime::ns>(intern_var("clojure.core", "*ns*").expect_ok()->deref())
          ->to_string());
      codegen::processor cg_prc{ *this, wrapped_exprs, module, codegen::compilation_target::ns };
      write_module(current_module, cg_prc.declaration_str());
      write_module(fmt::format("{}__init", current_module), cg_prc.module_init_str(current_module));
    }

    assert(ret);
    return ret;
  }

  native_vector<analyze::expression_ptr>
  context::analyze_string(native_persistent_string_view const &code, native_bool const eval)
  {
    profile::timer timer{ "rt analyze_string" };
    read::lex::processor l_prc{ code };
    read::parse::processor p_prc{ *this, l_prc.begin(), l_prc.end() };

    native_vector<analyze::expression_ptr> ret{};
    for(auto const &form : p_prc)
    {
      auto const expr(an_prc.analyze(form.expect_ok(), analyze::expression_type::statement));
      if(eval)
      {
        evaluate::eval(*this, jit_prc, expr.expect_ok());
      }
      ret.emplace_back(expr.expect_ok());
    }

    return ret;
  }

  result<void, native_persistent_string>
  context::load_module(native_persistent_string_view const &module)
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

    binding_scope preserve{ *this };

    try
    {
      result<void, native_persistent_string> res{ ok() };
      if(absolute_module.find('$') == native_persistent_string::npos)
      {
        res = module_loader.load_ns(absolute_module);
      }
      else
      {
        res = module_loader.load(absolute_module);
      }
      return res;
    }
    catch(std::exception const &e)
    {
      return err(e.what());
    }
    catch(object_ptr const &e)
    {
      return err(detail::to_string(e));
    }
  }

  result<void, native_persistent_string>
  context::compile_module(native_persistent_string_view const &module)
  {
    module_dependencies.clear();

    binding_scope preserve{ *this,
                            obj::persistent_hash_map::create_unique(
                              std::make_pair(compile_files_var, obj::boolean::true_const()),
                              std::make_pair(current_module_var, make_box(module))) };

    return load_module(fmt::format("/{}", module));
  }

  void context::write_module(native_persistent_string_view const &module,
                             native_persistent_string_view const &contents) const
  {
    profile::timer timer{ "write_module" };
    boost::filesystem::path const dir{ output_dir };
    if(!boost::filesystem::exists(dir))
    {
      boost::filesystem::create_directories(dir);
    }

    /* TODO: This needs to go into sub directories. Also, we should register these modules with
     * the loader upon writing. */
    {
      std::ofstream ofs{ fmt::format("{}/{}.cpp", module::module_to_path(output_dir), module) };
      ofs << contents;
      ofs.flush();
    }
  }

  native_persistent_string context::unique_string()
  {
    return unique_string("G_");
  }

  native_persistent_string context::unique_string(native_persistent_string_view const &prefix)
  {
    static std::atomic_size_t index{ 1 };
    return fmt::format(FMT_COMPILE("{}_{}"), prefix.data(), index++);
  }

  obj::symbol context::unique_symbol()
  {
    return unique_symbol("G_");
  }

  obj::symbol context::unique_symbol(native_persistent_string_view const &prefix)
  {
    return { "", unique_string(prefix) };
  }

  void context::dump() const
  {
    std::cout << "context dump" << std::endl;
    auto locked_namespaces(namespaces.rlock());
    for(auto const &p : *locked_namespaces)
    {
      std::cout << "  " << p.second->name->to_string() << std::endl;
      auto locked_vars(p.second->vars.rlock());
      for(auto const &vp : (*locked_vars)->data)
      {
        auto const v(expect_object<var>(vp.second));
        if(v->deref() == nullptr)
        {
          std::cout << "    " << v->to_string() << " = nil" << std::endl;
        }
        else
        {
          std::cout << "    " << v->to_string() << " = " << detail::to_string(v->deref())
                    << std::endl;
        }
      }
    }
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
    profile::timer timer{ "intern_var" };
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

  /* TODO: Swap these. The other one makes a symbol anyway. */
  result<obj::keyword_ptr, native_persistent_string>
  context::intern_keyword(obj::symbol const &sym, bool const resolved)
  {
    return intern_keyword(sym.ns, sym.name, resolved);
  }

  result<obj::keyword_ptr, native_persistent_string>
  context::intern_keyword(native_persistent_string_view const &ns,
                          native_persistent_string_view const &name,
                          bool const resolved)
  {
    profile::timer timer{ "rt intern_keyword" };
    obj::symbol sym{ ns, name };
    if(!resolved)
    {
      /* The ns will be an ns alias. */
      if(!ns.empty())
      {
        auto const resolved_ns(resolve_ns(make_box<obj::symbol>(ns)));
        if(resolved_ns.is_none())
        {
          return err(fmt::format("Unable to resolve ns for keyword: {}", ns));
        }
        sym.set_ns(resolved_ns.unwrap()->name->name);
      }
      else
      {
        auto const current_ns(expect_object<jank::runtime::ns>(current_ns_var->deref()));
        sym.set_ns(current_ns->name->name);
      }
    }

    auto locked_keywords(keywords.wlock());
    auto const found(locked_keywords->find(sym));
    if(found != locked_keywords->end())
    {
      return found->second;
    }

    auto const res(locked_keywords->emplace(sym, make_box<obj::keyword>(sym)));
    return res.first->second;
  }

  object_ptr context::macroexpand1(object_ptr const o)
  {
    profile::timer timer{ "rt macroexpand1" };
    return visit_object(
      [this](auto const typed_o) -> object_ptr {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(!std::same_as<T, obj::persistent_list>)
        {
          return typed_o;
        }
        else
        {
          if(typed_o->data.empty())
          {
            return typed_o;
          }

          auto const first_sym_obj(typed_o->data.first().unwrap());
          if(first_sym_obj->type != object_type::symbol)
          {
            return typed_o;
          }

          auto const var(find_var(expect_object<obj::symbol>(first_sym_obj)));
          /* None means it's not a var, so not a macro. No meta means no :macro set. */
          if(var.is_none() || var.unwrap()->meta.is_none())
          {
            return typed_o;
          }

          auto const meta(var.unwrap()->meta.unwrap());
          auto const found_macro(get(meta, intern_keyword("", "macro", true).expect_ok()));
          if(!found_macro || !detail::truthy(found_macro))
          {
            return typed_o;
          }

          auto const args(make_box<obj::persistent_list>(
            typed_o->data.rest().cons(obj::nil::nil_const()).cons(typed_o)));
          return apply_to(var.unwrap()->deref(), args);
        }
      },
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

  object_ptr context::print(object_ptr const o)
  {
    auto const s(detail::to_string(o));
    std::fwrite(s.data(), 1, s.size(), stdout);
    return obj::nil::nil_const();
  }

  object_ptr context::print(object_ptr const o, object_ptr const more)
  {
    visit_object(
      [o](auto const typed_more) {
        using T = typename decltype(typed_more)::value_type;

        if constexpr(behavior::sequenceable<T>)
        {
          fmt::memory_buffer buff;
          auto inserter(std::back_inserter(buff));
          detail::to_string(o, buff);
          detail::to_string(typed_more->first(), buff);
          for(auto it(typed_more->next_in_place()); it != nullptr; it = it->next_in_place())
          {
            fmt::format_to(inserter, " ");
            detail::to_string(it->first(), buff);
          }
          std::fwrite(buff.data(), 1, buff.size(), stdout);
        }
        else
        {
          throw std::runtime_error{ fmt::format("expected a sequence: {}",
                                                typed_more->to_string()) };
        }
      },
      more);
    return obj::nil::nil_const();
  }

  object_ptr context::println(object_ptr const more)
  {
    visit_object(
      [](auto const typed_more) {
        using T = typename decltype(typed_more)::value_type;

        if constexpr(std::same_as<T, obj::nil>)
        {
          std::putc('\n', stdout);
        }
        else if constexpr(behavior::sequenceable<T>)
        {
          fmt::memory_buffer buff;
          auto inserter(std::back_inserter(buff));
          detail::to_string(typed_more->first(), buff);
          for(auto it(typed_more->next_in_place()); it != nullptr; it = it->next_in_place())
          {
            fmt::format_to(inserter, " ");
            detail::to_string(it->first(), buff);
          }
          std::fwrite(buff.data(), 1, buff.size(), stdout);
          std::putc('\n', stdout);
        }
        else
        {
          throw std::runtime_error{ fmt::format("expected a sequence: {}",
                                                typed_more->to_string()) };
        }
      },
      more);
    return obj::nil::nil_const();
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
    rt_ctx.pop_thread_bindings().expect_ok();
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
      return err(fmt::format("invalid thread binding map: {}", detail::to_string(bindings)));
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

    for(auto it(bindings->fresh_seq()); it != nullptr; it = it->next_in_place())
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
