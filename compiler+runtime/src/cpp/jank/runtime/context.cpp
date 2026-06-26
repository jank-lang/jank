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
  /* NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) */
  context *__rt_ctx{};

  context::context()
    /* We want to initialize __rt_ctx ASAP so other code can start using it. */
    : binary_version{ (__rt_ctx = this, util::binary_version()) }
    , jit_prc{ binary_version }
  {
    intern_ns(make_box<obj::symbol>("cpp"));
    auto const core(intern_ns(make_box<obj::symbol>("clojure.core")));

    auto const file_sym(make_box<obj::symbol>("*file*"));
    current_file_var = core->intern_var(file_sym);
    current_file_var->bind_root(make_box(read::no_source_path));
    current_file_var->dynamic.store(true);

    auto const ns_sym(make_box<obj::symbol>("*ns*"));
    current_ns_var = core->intern_var(ns_sym);
    current_ns_var->bind_root(core);
    current_ns_var->dynamic.store(true);

    auto const stream_sym(make_box<obj::symbol>("stream*"));
    stream_var = core->intern_var(stream_sym);

    auto const out_sym(make_box<obj::symbol>("*out*"));
    current_out_var = core->intern_var(out_sym);
    current_out_var->dynamic.store(true);

    auto const err_sym(make_box<obj::symbol>("*err*"));
    current_err_var = core->intern_var(err_sym);
    current_err_var->dynamic.store(true);

    auto const compile_files_sym(make_box<obj::symbol>("*compile-files*"));
    compile_files_var = core->intern_var(compile_files_sym);
    compile_files_var->bind_root(jank_false);
    compile_files_var->dynamic.store(true);

    auto const loaded_libs_sym(make_box<obj::symbol>("*loaded-libs*"));
    loaded_libs_var = core->intern_var(loaded_libs_sym);
    loaded_libs_var->bind_root(make_box<obj::atom>(
      obj::persistent_sorted_set::create_from_seq(make_box<obj::persistent_vector>(
        runtime::detail::native_persistent_vector{ make_box<obj::symbol>("clojure.core") }))));
    loaded_libs_var->dynamic.store(true);

    auto const assert_sym(make_box<obj::symbol>("*assert*"));
    assert_var = core->intern_var(assert_sym);
    assert_var->bind_root(jank_true);
    assert_var->dynamic.store(true);

    __rt_ctx->intern_var("clojure.core", "*read-eval*")
      .expect_ok()
      ->bind_root(jank_true)
      ->set_dynamic(true);

    auto const command_line_args_sym(make_box<obj::symbol>("*command-line-args*"));
    auto const command_line_args_var{ core->intern_var(command_line_args_sym) };
    command_line_args_var->bind_root({});
    command_line_args_var->dynamic.store(true);

    /* These are not actually interned. They're extra private. */
    current_module_var
      = make_box<runtime::var>(core, make_box<obj::symbol>("*current-module*"))->set_dynamic(true);
    no_recur_var
      = make_box<runtime::var>(core, make_box<obj::symbol>("*no-recur*"))->set_dynamic(true);
    gensym_env_var
      = make_box<runtime::var>(core, make_box<obj::symbol>("*gensym-env*"))->set_dynamic(true);

    /* This won't be set until clojure.core is loaded. */
    auto const in_ns_sym(make_box<obj::symbol>("clojure.core/in-ns"));
    in_ns_var = intern_var(in_ns_sym).expect_ok();

    /* These will be set by any connected REPL, so we want to make sure they have
     * an active binding. */
    auto const star1(core->intern_var(make_box<obj::symbol>("*1"))->set_dynamic(true));
    auto const star2(core->intern_var(make_box<obj::symbol>("*2"))->set_dynamic(true));
    auto const star3(core->intern_var(make_box<obj::symbol>("*3"))->set_dynamic(true));
    auto const stare(core->intern_var(make_box<obj::symbol>("*e"))->set_dynamic(true));

    push_thread_bindings(obj::persistent_hash_map::create_unique(
                           std::make_pair(current_ns_var, current_ns_var->deref()),
                           std::make_pair(star1, jank_nil),
                           std::make_pair(star2, jank_nil),
                           std::make_pair(star3, jank_nil),
                           std::make_pair(stare, jank_nil)))
      .expect_ok();
  }

  obj::symbol_ref context::qualify_symbol(obj::symbol_ref const sym) const
  {
    obj::symbol_ref qualified_sym{ sym };
    if(qualified_sym->ns.empty())
    {
      auto const current_ns(expect_object<ns>(current_ns_var->deref()));
      qualified_sym = make_box<obj::symbol>(current_ns->name->name, sym->name);
    }
    else
    {
      auto const resolved_ns(__rt_ctx->resolve_ns(make_box<obj::symbol>(qualified_sym->ns)));

      if(resolved_ns.is_some())
      {
        qualified_sym = make_box<obj::symbol>(resolved_ns->name->name, sym->name);
      }
    }
    return qualified_sym;
  }

  var_ref context::find_var(obj::symbol_ref const sym)
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

  object_ref context::read_string(jtl::immutable_string const &code,
                                  object_ref const reader_opts,
                                  u64 const nth_form)
  {
    profile::timer const timer{ "rt read_string" };
    static auto const unknown_kw{ __rt_ctx->intern_keyword("unknown").expect_ok() };
    auto const read_eval_var{ __rt_ctx->find_var("clojure.core", "*read-eval*") };
    auto const read_eval_enabled{ read_eval_var.is_some()
                                  && !equal(read_eval_var->deref(), unknown_kw) };

    if(!read_eval_enabled)
    {
      throw std::runtime_error{
        read_eval_var.is_nil()
          ? "Reading is disallowed when `clojure.core/*read-eval*` is unbound."
          : "Reading is disallowed when `clojure.core/*read-eval*` is bound to `:unknown`."
      };
    }

    /* The `read` & similar `clojure.core` functions in Clojure, don't read in reader
     * conditionals by default. This behavior can be configured using the `:read-cond`
     * reader option using the following values:
     *   - :allow - Reader conditionals are read in and processed by the parser.
     *   - :preserve - Reader conditionals are read in but the exact form is preserved,
     *                 along with the unsupported reader conditional features. */
    bool allow_reader_conditional{ false };
    bool in_preservation_mode{};
    /* The supported reader conditional features can be extended using the `:features`
     * reader option. */
    object_ref extended_features{};

    if(!is_map(reader_opts))
    {
      throw std::runtime_error{ util::format(
        "The reader options need to be a map. Found a {} instead.",
        runtime::object_type_str(reader_opts.get_type())) };
    }

    static auto const read_cond_kw{ __rt_ctx->intern_keyword("", "read-cond").expect_ok() };
    auto const read_cond{ get(reader_opts, read_cond_kw) };

    if(read_cond.is_some())
    {
      auto const reader_cond{ try_object<obj::keyword>(read_cond) };
      static auto const preserve_kw{ __rt_ctx->intern_keyword("", "preserve").expect_ok() };
      static auto const allow_kw{ __rt_ctx->intern_keyword("", "allow").expect_ok() };

      if(reader_cond == preserve_kw)
      {
        in_preservation_mode = true;
        allow_reader_conditional = true;
      }
      else if(reader_cond == allow_kw)
      {
        allow_reader_conditional = true;
      }
      else
      {
        throw std::runtime_error{ util::format(
          "Only :preserve or :allow modes are supported for :read-cond reader option. Found {} "
          "instead.",
          runtime::to_code_string(read_cond)) };
      }
    }

    static auto const features_kw{ __rt_ctx->intern_keyword("", "features").expect_ok() };
    auto const features{ get(reader_opts, features_kw) };

    if(features.is_some())
    {
      if(!is_set(features))
      {
        throw std::runtime_error{ util::format(
          "The :features reader option needs to be a set. Found a {} instead.",
          runtime::object_type_str(features.get_type())) };
      }

      extended_features = features;
    }

    /* When reading an arbitrary string, we don't want the last *current-file* to
     * be set as source file, so we need to bind it to nil. */
    binding_scope const preserve{ obj::persistent_hash_map::create_unique(
      std::make_pair(current_file_var, jank_nil)) };
    read::lex::processor l_prc{ code };
    read::parse::processor p_prc{ l_prc.begin(),
                                  l_prc.end(),
                                  extended_features,
                                  allow_reader_conditional,
                                  in_preservation_mode };
    auto const read_entire_stream{ nth_form == std::numeric_limits<u64>::max() };
    u64 count{};
    static auto const eof_kw{ __rt_ctx->intern_keyword("", "eof").expect_ok() };
    static auto const eof_throw_kw{ __rt_ctx->intern_keyword("", "eofthrow").expect_ok() };
    auto const eof_value{ get(reader_opts, eof_kw) };
    auto const throw_on_eof{ equal(eof_value, eof_throw_kw) };
    bool eof_found{ throw_on_eof };
    object_ref ret{};

    for(auto const &form : p_prc)
    {
      if(nth_form <= count)
      {
        break;
      }

      if(form.expect_ok().is_none())
      {
        eof_found = true;
        ret = eof_value;
      }
      else
      {
        /* In Clojure, the reader doesn't count comments, etc. as forms. */
        ++count;
        eof_found = false;
        ret = form.expect_ok().unwrap().ptr;
      }
    }

    if(!read_entire_stream && count < nth_form)
    {
      eof_found = true;
      ret = eof_value;
    }

    if(throw_on_eof && eof_found)
    {
      throw std::runtime_error{ "EOF reached while reading. To override this behavior, provide a "
                                "return value for the EOF case via the `:eof` reader option." };
    }

    return ret;
  }

  object_ref context::read_string(jtl::immutable_string const &code, object_ref const reader_opts)
  {
    /* The Clojure `read` & similar functions always read the first form in a string
     * containing more than one form. */
    return read_string(code, reader_opts, /* nth_form */ 1);
  }

  object_ref context::read_string(jtl::immutable_string const &code)
  {
    static auto const eof_kw{ __rt_ctx->intern_keyword("", "eof").expect_ok() };
    static auto const eof_throw_kw{ __rt_ctx->intern_keyword("", "eofthrow").expect_ok() };
    static auto const default_reader_opts{ obj::persistent_array_map::create_unique(eof_kw,
                                                                                    eof_throw_kw) };
    return read_string(code, default_reader_opts);
  }

  object_ref context::forcefully_read_string(jtl::immutable_string const &code)
  {
    static auto const read_eval_enabled_var{ __rt_ctx->find_var("clojure.core", "*read-eval*") };
    binding_scope const bindings{ obj::persistent_hash_map::create_unique(
      std::make_pair(read_eval_enabled_var, jank_true)) };
    return read_string(code);
  }

  jtl::result<void, error_ref>
  context::load_module(jtl::immutable_string const &module, module::origin const ori)
  {
    auto const ns(current_ns());

    /* When we load a module, the `*ns*` var is still set to the previous module.
     * In the `clojure.core/ns` macro, `in-ns` is called that sets the value of the
     * current ns to the module being loaded. To avoid overwriting the previous `ns` value, `current_ns_var`
     * binding is pushed in the context, and then `in-ns` sets the value of `*ns*` var in
     * the new binding scope. */
    binding_scope const preserve{ obj::persistent_hash_map::create_unique(
      std::make_pair(current_ns_var, ns),
      std::make_pair(current_module_var, make_box(module))) };

    try
    {
      return module_loader.load(module, ori);
    }
    catch(std::exception const &e)
    {
      return error::runtime_unable_to_load_module(e.what());
    }
    catch(object_ref const e)
    {
      return error::runtime_unable_to_load_module(runtime::to_code_string(e));
    }
    catch(error_ref const e)
    {
      return e;
    }
  }

  jtl::immutable_string context::unique_namespaced_string() const
  {
    return unique_namespaced_string("G_");
  }

  jtl::immutable_string context::unique_namespaced_string(jtl::immutable_string const &prefix) const
  {
    static jtl::immutable_string const dot{ "\\." };
    auto const ns{ current_ns() };
    return util::format("{}-{}-{}",
                        runtime::munge_and_replace(ns->name->get_name(), dot, "_"),
                        prefix.c_str(),
                        ++ns->symbol_counter);
  }

  jtl::immutable_string context::unique_string() const
  {
    return unique_string("G_");
  }

  jtl::immutable_string context::unique_string(jtl::immutable_string const &prefix) const
  {
    auto const ns{ current_ns() };
    return util::format("{}-{}", prefix.c_str(), ++ns->symbol_counter);
  }

  obj::symbol_ref context::unique_symbol() const
  {
    return unique_symbol("G-");
  }

  obj::symbol_ref context::unique_symbol(jtl::immutable_string const &prefix) const
  {
    return make_box<obj::symbol>("", unique_namespaced_string(prefix));
  }

  ns_ref context::intern_ns(jtl::immutable_string const &name)
  {
    return intern_ns(make_box<obj::symbol>(name));
  }

  ns_ref context::intern_ns(obj::symbol_ref const sym)
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

    auto const result(locked_namespaces->emplace(sym, make_box<ns>(sym)));
    return result.first->second;
  }

  ns_ref context::remove_ns(obj::symbol_ref const sym)
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

  ns_ref context::find_ns(obj::symbol_ref const sym)
  {
    auto locked_namespaces(namespaces.rlock());
    auto const found(locked_namespaces->find(sym));
    if(found != locked_namespaces->end())
    {
      return found->second;
    }
    return {};
  }

  ns_ref context::resolve_ns(obj::symbol_ref const target)
  {
    auto const ns(current_ns());
    auto alias(ns->find_alias(target));
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

  native_vector<ns_ref> context::all_ns() const
  {
    auto locked_namespaces(namespaces.rlock());
    native_vector<ns_ref> ret;
    ret.reserve(locked_namespaces->size());
    for(auto const p : *locked_namespaces)
    {
      /* This isn't a real ns. */
      if(p.first->name == "cpp")
      {
        continue;
      }

      ret.emplace_back(p.second);
    }
    return ret;
  }

  jtl::result<var_ref, jtl::immutable_string>
  context::intern_var(jtl::immutable_string const &qualified_name)
  {
    return intern_var(make_box<obj::symbol>(qualified_name));
  }

  jtl::result<var_ref, jtl::immutable_string>
  context::intern_var(jtl::immutable_string const &ns, jtl::immutable_string const &name)
  {
    return intern_var(make_box<obj::symbol>(ns, name));
  }

  jtl::result<var_ref, jtl::immutable_string>
  context::intern_var(obj::symbol_ref const qualified_name)
  {
    profile::timer const timer{ "intern_var" };
    if(qualified_name->ns.empty())
    {
      return err(
        util::format("Can't intern var. Sym isn't qualified: {}", qualified_name->to_string()));
    }

    obj::symbol_ref const ns_sym{ make_box<obj::symbol>(qualified_name->ns) };
    ns_ref found_ns;
    {
      auto locked_namespaces(namespaces.rlock());
      auto const found{ locked_namespaces->find(ns_sym) };
      if(found != locked_namespaces->end())
      {
        found_ns = found->second;
      }
    }
    if(found_ns.is_nil())
    {
      found_ns = intern_ns(ns_sym);
    }

    return ok(found_ns->intern_var(qualified_name));
  }

  jtl::result<var_ref, jtl::immutable_string>
  context::intern_owned_var(jtl::immutable_string const &ns, jtl::immutable_string const &name)
  {
    return intern_owned_var(make_box<obj::symbol>(ns, name));
  }

  jtl::result<var_ref, jtl::immutable_string>
  context::intern_owned_var(jtl::immutable_string const &qualified_name)
  {
    return intern_owned_var(make_box<obj::symbol>(qualified_name));
  }

  jtl::result<var_ref, jtl::immutable_string>
  context::intern_owned_var(obj::symbol_ref const qualified_sym)
  {
    /* TODO: Clean up duplication between this and intern_var. */
    profile::timer const timer{ "intern_var" };
    if(qualified_sym->ns.empty())
    {
      return err(
        util::format("Can't intern var. Sym isn't qualified: {}", qualified_sym->to_string()));
    }

    obj::symbol_ref const ns_sym{ make_box<obj::symbol>(qualified_sym->ns) };
    ns_ref found_ns;
    {
      auto locked_namespaces(namespaces.rlock());
      auto const found{ locked_namespaces->find(ns_sym) };
      if(found != locked_namespaces->end())
      {
        found_ns = found->second;
      }
    }
    if(found_ns.is_nil())
    {
      found_ns = intern_ns(ns_sym);
    }

    return ok(found_ns->intern_owned_var(qualified_sym));
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
      locked_keywords->emplace(s, make_box<obj::keyword>(runtime::detail::must_be_interned{}, s)));
    return res.first->second;
  }

  object_ref context::macroexpand1(object_ref const o)
  {
    profile::timer const timer{ "rt macroexpand1" };
    return visit_seqable(
      [this](auto const typed_o) -> object_ref {
        using T = typename jtl::decay_t<decltype(typed_o)>::value_type;

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

          auto const resolved_sym(qualify_symbol(first_sym_obj));
          auto const var(find_var(resolved_sym));
          if(var.is_nil())
          {
            return typed_o;
          }

          auto const meta(var->get_meta());
          auto const found_macro(get(meta, intern_keyword("", "macro", true).expect_ok()));
          if(found_macro.is_nil() || !truthy(found_macro))
          {
            return typed_o;
          }

          /* TODO: Provide &env. */
          auto const args(cons(cons(rest(typed_o), {}), typed_o));
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
      if(source != read::source::unknown())
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

  context::binding_scope::binding_scope()
  {
    __rt_ctx->push_thread_bindings().expect_ok();
  }

  context::binding_scope::binding_scope(obj::persistent_hash_map_ref const bindings)
  {
    __rt_ctx->push_thread_bindings(bindings).expect_ok();
  }

  context::binding_scope::~binding_scope()
  {
    __rt_ctx->pop_thread_bindings();
  }

  jtl::string_result<void> context::push_thread_bindings()
  {
    auto bindings(obj::persistent_hash_map::empty());
    {
      auto const thread_id{ std::this_thread::get_id() };
      auto tbfs_map(thread_binding_frames.rlock());
      auto const tbfs(thread_binding_frames->find(thread_id));
      if(tbfs != thread_binding_frames->end() && !tbfs->second.empty())
      {
        bindings = tbfs->second.front().bindings;
      }
    }

    return push_thread_bindings(bindings);
  }

  jtl::string_result<void> context::push_thread_bindings(object_ref const bindings)
  {
    if(bindings.get_type() != object_type::persistent_hash_map)
    {
      return err(util::format("invalid thread binding map (must be hash map): {}",
                              runtime::to_code_string(bindings)));
    }

    return push_thread_bindings(expect_object<obj::persistent_hash_map>(bindings));
  }

  jtl::string_result<void>
  context::push_thread_bindings(obj::persistent_hash_map_ref const bindings)
  {
    thread_binding_frame frame{ obj::persistent_hash_map::empty() };
    auto const thread_id{ std::this_thread::get_id() };
    auto tbfs_map(thread_binding_frames.wlock());
    auto &tbfs{ (*tbfs_map)[thread_id] };
    if(!tbfs.empty())
    {
      frame.bindings = tbfs.front().bindings;
    }

    for(auto it(bindings->fresh_seq()); it.is_some(); it = it->next_in_place())
    {
      auto const entry(it->first());
      auto const var(try_object<var>(entry->data[0]));
      if(!var->dynamic.load())
      {
        return err(
          util::format("Can't dynamically bind non-dynamic var: {}", var->to_code_string()));
      }

      /* XXX: Once this is set to true, here, it's never unset. */
      var->thread_bound.store(true);

      /* The binding may already be a thread binding if we're just pushing the previous
       * bindings again to give a scratch pad for some upcoming code. */
      if(entry->data[1].get_type() == object_type::var_thread_binding)
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

    tbfs.push_front(jtl::move(frame));
    return ok();
  }

  void context::pop_thread_bindings()
  {
    auto const thread_id{ std::this_thread::get_id() };
    auto tbfs_map(thread_binding_frames.wlock());
    auto &tbfs{ (*tbfs_map)[thread_id] };
    if(tbfs.empty())
    {
      return;
    }

    tbfs.pop_front();
  }

  obj::persistent_hash_map_ref context::get_thread_bindings() const
  {
    auto const thread_id{ std::this_thread::get_id() };
    auto tbfs_map(thread_binding_frames.rlock());
    auto const tbfs{ tbfs_map->find(thread_id) };
    if(tbfs == tbfs_map->end() || tbfs->second.empty())
    {
      return obj::persistent_hash_map::empty();
    }
    return tbfs->second.front().bindings;
  }
}
