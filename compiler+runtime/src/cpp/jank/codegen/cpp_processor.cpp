#include <CppInterOp/Compatibility.h>
#include <CppInterOp/CppInterOp.h>

#include <jank/analyze/visit.hpp>
#include <jank/analyze/cpp_util.hpp>
#include <jank/ir/processor.hpp>
#include <jank/ir/visit.hpp>
#include <jank/codegen/cpp_processor.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/munge.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/core/meta.hpp>
#include <jank/runtime/core/call.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/sequence_range.hpp>
#include <jank/util/escape.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/util/clang_format.hpp>

namespace jank::codegen
{
  using namespace analyze::cpp_util;
  using namespace runtime;

  jtl::ref<ir::function>
  find_function(jtl::ref<ir::module> const module, jtl::immutable_string const &function_name)
  {
    for(auto const &fn : module->functions)
    {
      if(fn.name == function_name)
      {
        return &fn;
      }
    }
    jank_panic_fmt("Unable to find IR function '{}'.", function_name);
  }

  struct builder
  {
    builder(jtl::ref<ir::module> const module, jtl::immutable_string const &function_name)
      : module{ module }
      , function{ find_function(module, function_name) }
    {
    }

    void enter_block(ir::identifier const &block)
    {
      block_index = function->find_block(block);
      instruction_index = 0;
      seen_blocks.emplace(block);
    }

    void next_instruction()
    {
      ++instruction_index;
    }

    void defer(jtl::immutable_string const &context, jtl::immutable_string const &binding)
    {
      deferred_bindings.emplace_back(context, binding);
    }

    jtl::immutable_string declaration_str() const
    {
      native_transient_string declaration;
      declaration.reserve(cpp_raw_buffer.size() + module_header_buffer.size() + deps_buffer.size()
                          + header_buffer.size() + body_buffer.size() + footer_buffer.size());
      declaration += jtl::immutable_string_view{ cpp_raw_buffer.data(), cpp_raw_buffer.size() };
      declaration
        += jtl::immutable_string_view{ module_header_buffer.data(), module_header_buffer.size() };
      declaration += jtl::immutable_string_view{ deps_buffer.data(), deps_buffer.size() };
      declaration += jtl::immutable_string_view{ header_buffer.data(), header_buffer.size() };
      declaration += jtl::immutable_string_view{ body_buffer.data(), body_buffer.size() };
      declaration += jtl::immutable_string_view{ footer_buffer.data(), footer_buffer.size() };

      return declaration;
    }

    jtl::immutable_string expression_str()
    {
      if(!expression_buffer.empty())
      {
        return { expression_buffer.data(), expression_buffer.size() };
      }

      auto const ret_tmp{ munge(__rt_ctx->unique_string("fnexpr")) };
      util::format_to(expression_buffer, "auto const {}(", ret_tmp);
      util::format_to(expression_buffer, "_jank_fn({})", module->arity_flags);
      util::format_to(expression_buffer, ");");

      for(auto const &arity : module->root_fn_expr->arities)
      {
        auto const param_count{ arity.fn_ctx->param_count };
        util::format_to(expression_buffer,
                        "{}->arity_{} = &{}_{};",
                        ret_tmp,
                        param_count,
                        munge(module->root_fn_expr->unique_name),
                        param_count);
      }

      util::format_to(expression_buffer, "{}", ret_tmp);

      return { expression_buffer.data(), expression_buffer.size() };
    }

    jtl::ref<ir::module> module;
    jtl::ref<ir::function> function;

    jtl::string_builder cpp_def_buffer{};
    jtl::string_builder cpp_raw_buffer{};
    jtl::string_builder module_header_buffer{};
    jtl::string_builder deps_buffer{};
    jtl::string_builder header_buffer{};
    jtl::string_builder body_buffer{};
    jtl::string_builder footer_buffer{};
    jtl::string_builder expression_buffer{};

    usize block_index{}, instruction_index{};
    native_vector<std::pair<jtl::immutable_string, jtl::immutable_string>> deferred_bindings;
    native_set<ir::identifier> seen_blocks;
  };

  using identifier = ir::identifier;


  static folly::Synchronized<
    native_unordered_map<object_ref, identifier, std::hash<object_ref>, very_equal_to_with_meta>>
    /* NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) */
    global_constants;
  static folly::Synchronized<native_unordered_map<jtl::immutable_string, identifier>>
    /* NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) */
    global_vars;

  static identifier lift_constant(identifier const &name, object_ref const o, builder &b)
  {
    if(b.module->target == compilation_target::eval)
    {
      auto locked_global_constants{ global_constants.wlock() };
      auto const found{ locked_global_constants->find(o) };
      if(found != locked_global_constants->end())
      {
        return found->second;
      }

      /* We want to use this global directly, since it's already in memory. We need the
       * GC to hang onto it, though, so we allocate an uncollectable pointer to hold
       * our object. */
      [[maybe_unused]]
      auto * const root{ new(NoGC) object *{ o.raw() } };
      auto const type{ literal_type(o) };
      auto const ptr{ static_cast<void *>(o.raw()) };
      jtl::immutable_string fmt_str;
      if(o.is_nil())
      {
        fmt_str = "jank::runtime::jank_nil";
      }
      else if(is_typed_object(type))
      {
        if(runtime::detail::is_tagged_pointer(ptr))
        {
          fmt_str = util::format("{}{ (void*){} }",
                                 get_qualified_type_name(type),
                                 runtime::detail::as_pointer(ptr));
        }
        else if(runtime::detail::is_tagged_small_int(ptr))
        {
          fmt_str = util::format("{}{ {} }",
                                 get_qualified_type_name(type),
                                 runtime::detail::as_integer(ptr));
        }
        else if(runtime::detail::is_tagged_small_real(ptr))
        {
          fmt_str = util::format("{}{ {} }",
                                 get_qualified_type_name(type),
                                 runtime::detail::as_real(ptr));
        }
      }
      else
      {
        fmt_str = util::format("{}{ (void*){} }", get_qualified_type_name(type), ptr);
      }

      /* TODO: Not a fan of this. Move into global? Init with uncollectable ptr. */
      locked_global_constants->emplace(o, fmt_str);
      return fmt_str;
    }
    return name;
  }

  static jtl::immutable_string lift_var(jtl::immutable_string const &qualified_var, builder &b)
  {
    if(b.module->target == compilation_target::eval)
    {
      auto locked_global_vars{ global_vars.wlock() };
      auto const found{ locked_global_vars->find(qualified_var) };
      if(found != locked_global_vars->end())
      {
        return found->second;
      }

      auto const var{ __rt_ctx->intern_var(qualified_var).expect_ok() };

      /* We want to use this global directly, since it's already in memory. We need the
       * GC to hang onto it, though, so we allocate an uncollectable pointer to hold
       * our object. */
      [[maybe_unused]]
      auto const root{ new(NoGC) runtime::var *{ reinterpret_cast<runtime::var *>(var.ptr()) } };
      auto const fmt_str{ util::format(
        "jank::runtime::var_ref{ reinterpret_cast<jank::runtime::var*>({}) }",
        static_cast<void *>(var.ptr())) };
      locked_global_vars->emplace(qualified_var, fmt_str);
      return fmt_str;
    }

    return b.module->lifted_vars.at(qualified_var).name;
  }

  void gen(ir::function const &fn, builder &b);

  namespace detail
  {
    static bool should_gen_meta(object_ref const meta)
    {
      return meta.is_some() && !is_empty(meta);
    }

    static void gen_constant(object_ref const o, jtl::string_builder &buffer)
    {
      visit_object(
        [&](auto const typed_o) {
          using T = typename decltype(typed_o)::value_type;

          if constexpr(std::same_as<T, obj::nil>)
          {
            util::format_to(buffer, "jank::runtime::jank_nil");
          }
          else if constexpr(std::same_as<T, obj::boolean>)
          {
            if(typed_o->data)
            {
              util::format_to(buffer, "jank::runtime::jank_true");
            }
            else
            {
              util::format_to(buffer, "jank::runtime::jank_false");
            }
          }
          else if constexpr(jtl::is_any_same<T, obj::integer, obj::small_integer>)
          {
            if(static_cast<i32>(typed_o->data) == typed_o->data)
            {
              util::format_to(buffer, "_jank_small_int({})", typed_o->data);
            }
            else
            {
              util::format_to(buffer, "_jank_int({})", typed_o->data);
            }
          }
          else if constexpr(jtl::is_any_same<T, obj::real, obj::small_real>)
          {
            util::format_to(buffer, "_jank_small_real({})", typed_o->data);
          }
          else if constexpr(std::same_as<T, obj::big_integer>)
          {
            util::format_to(buffer,
                            "jank::runtime::make_box<jank::runtime::obj::big_integer>(\"{}\")",
                            typed_o->to_string());
          }
          else if constexpr(std::same_as<T, obj::big_decimal>)
          {
            util::format_to(buffer,
                            "jank::runtime::make_box<jank::runtime::obj::big_decimal>(\"{}\")",
                            typed_o->to_string());
          }
          else if constexpr(std::same_as<T, obj::ratio>)
          {
            util::format_to(buffer,
                            "jank::runtime::obj::ratio::create({}, {})",
                            typed_o->data.numerator,
                            typed_o->data.denominator);
          }
          else if constexpr(std::same_as<T, obj::symbol>)
          {
            util::format_to(buffer, "_jank_symbol( ");
            if(should_gen_meta(typed_o->get_meta()))
            {
              util::format_to(buffer,
                              "\"{}\",",
                              util::escape(typed_o->get_meta().to_code_string()));
            }
            util::format_to(buffer, R"("{}", "{}"))", typed_o->ns, typed_o->name);
          }
          else if constexpr(std::same_as<T, obj::character>)
          {
            util::format_to(buffer,
                            R"(jank::runtime::make_box<jank::runtime::obj::character>("{}"))",
                            util::escape(typed_o->to_string()));
          }
          else if constexpr(std::same_as<T, obj::keyword>)
          {
            util::format_to(buffer,
                            R"(_jank_keyword("{}", "{}"))",
                            typed_o->sym->ns,
                            typed_o->sym->name);
          }
          else if constexpr(std::same_as<T, obj::re_pattern>)
          {
            util::format_to(buffer,
                            R"(jank::runtime::make_box<jank::runtime::obj::re_pattern>({}))",
                            /* We remove the # prefix here. */
                            typed_o->to_code_string().substr(1));
          }
          else if constexpr(std::same_as<T, obj::uuid>)
          {
            util::format_to(buffer,
                            R"(jank::runtime::make_box<jank::runtime::obj::uuid>("{}"))",
                            typed_o->to_string());
          }
          else if constexpr(std::same_as<T, obj::persistent_string>)
          {
            if(typed_o->data.empty())
            {
              util::format_to(buffer, "_jank_string()");
            }
            else
            {
              util::format_to(buffer, "_jank_string({})", typed_o->to_code_string());
            }
          }
          else if constexpr(std::same_as<T, obj::persistent_vector>)
          {
            util::format_to(buffer, "_jank_vec(");
            if(should_gen_meta(typed_o->get_meta()))
            {
              util::format_to(buffer,
                              "\"{}\",",
                              util::escape(typed_o->get_meta().to_code_string()));
            }
            util::format_to(buffer, "{}", typed_o->data.size());
            for(auto const &form : typed_o->data)
            {
              util::format_to(buffer, ", ");
              gen_constant(form, buffer);
              util::format_to(buffer, ".erase()");
            }
            util::format_to(buffer, ")");
          }
          else if constexpr(std::same_as<T, obj::persistent_list>)
          {
            util::format_to(buffer, "_jank_list(");
            if(should_gen_meta(typed_o->get_meta()))
            {
              util::format_to(buffer,
                              "\"{}\",",
                              util::escape(typed_o->get_meta().to_code_string()));
            }
            util::format_to(buffer, "{}", typed_o->data.size());
            for(auto const &form : typed_o->data)
            {
              util::format_to(buffer, ", ");
              gen_constant(form, buffer);
              util::format_to(buffer, ".erase()");
            }
            util::format_to(buffer, ")");
          }
          else if constexpr(std::same_as<T, obj::persistent_hash_set>)
          {
            util::format_to(buffer, "_jank_hset(");
            if(should_gen_meta(typed_o->get_meta()))
            {
              util::format_to(buffer,
                              "\"{}\",",
                              util::escape(typed_o->get_meta().to_code_string()));
            }
            util::format_to(buffer, "{}", typed_o->data.size());
            for(auto const &form : typed_o->data)
            {
              util::format_to(buffer, ", ");
              gen_constant(form, buffer);
              util::format_to(buffer, ".erase()");
            }
            util::format_to(buffer, ")");
          }
          else if constexpr(std::same_as<T, obj::persistent_array_map>)
          {
            util::format_to(buffer, "_jank_amap(");
            if(should_gen_meta(typed_o->get_meta()))
            {
              util::format_to(buffer,
                              "\"{}\",",
                              util::escape(typed_o->get_meta().to_code_string()));
            }
            util::format_to(buffer, "{}", typed_o->data.size());
            for(auto const &form : typed_o->data)
            {
              util::format_to(buffer, ", ");
              gen_constant(form.first, buffer);
              util::format_to(buffer, ".erase(), ");
              gen_constant(form.second, buffer);
              util::format_to(buffer, ".erase()");
            }
            util::format_to(buffer, ")");
          }
          else if constexpr(std::same_as<T, obj::persistent_hash_map>)
          {
            util::format_to(buffer, "_jank_hmap(");
            if(should_gen_meta(typed_o->get_meta()))
            {
              util::format_to(buffer,
                              "\"{}\",",
                              util::escape(typed_o->get_meta().to_code_string()));
            }
            util::format_to(buffer, "{}", typed_o->data.size());
            for(auto const &form : typed_o->data)
            {
              util::format_to(buffer, ", ");
              gen_constant(form.first, buffer);
              util::format_to(buffer, ".erase(), ");
              gen_constant(form.second, buffer);
              util::format_to(buffer, ".erase()");
            }
            util::format_to(buffer, ")");
          }
          /* Cons, etc. */
          else if constexpr(behavior::seqable<T>)
          {
            util::format_to(buffer, "_jank_list({}", sequence_length(typed_o));
            for(auto const it : make_sequence_range(typed_o))
            {
              util::format_to(buffer, ", ");
              gen_constant(it, buffer);
              util::format_to(buffer, ".erase()");
            }
            util::format_to(buffer, ")");
          }
          else
          {
            throw std::runtime_error{ util::format("Unimplemented constant codegen: {}\n",
                                                   typed_o->to_string()) };
          }
        },
        o);
    }
  }

  jtl::option<identifier> gen(ir::instruction_ref const &, builder &);

  void gen_until_jump(jtl::option<identifier> const &jump_block, builder &b)
  {
    while(b.instruction_index < b.function->blocks[b.block_index].instructions.size())
    {
      auto const current_inst{
        b.function->blocks[b.block_index].instructions[b.instruction_index]
      };

      if(current_inst->kind == ir::instruction_kind::jump)
      {
        auto const jump{ static_box_cast<ir::inst::jump>(current_inst) };
        if(b.seen_blocks.contains(jump->block))
        {
          gen(current_inst, b);
          break;
        }
      }

      gen(current_inst, b);

      if(current_inst->kind == ir::instruction_kind::jump)
      {
        auto const jump{ static_box_cast<ir::inst::jump>(current_inst) };
        if(jump_block.is_some() && jump->block == jump_block.unwrap())
        {
          break;
        }
      }
      else if(current_inst->kind == ir::instruction_kind::ret
              || current_inst->kind == ir::instruction_kind::throw_)
      {
        break;
      }
    }
  }

  jtl::option<identifier> gen(ir::inst::def_ref const &inst, builder &b)
  {
    b.next_instruction();
    /* def uses a var, but we don't lift it. Even if it's lifted by another usage,
     * it'll be re-interned here as an owned var. This needs to happen at the point
     * of the def, rather than prior (i.e. due to lifting), since there could be
     * some other var-related effects such as refer which need to happen before
     * def. */
    util::format_to(b.body_buffer,
                    "auto const {}(_jank_var_owned(\"{}\"));\n",
                    inst->name,
                    inst->qualified_var);

    /* Forward declarations just intern the var and evaluate to it. */
    if(inst->value.is_none())
    {
      util::format_to(b.body_buffer,
                      "{}->with_lazy_meta(\"{}\")->set_dynamic({});\n",
                      inst->name,
                      util::escape(inst->meta.to_code_string()),
                      inst->is_dynamic);
      return inst->name;
    }

    util::format_to(b.body_buffer,
                    "{}->bind_root({})->with_lazy_meta(\"{}\")->set_dynamic({});\n",
                    inst->name,
                    inst->value.unwrap(),
                    util::escape(inst->meta.to_code_string()),
                    inst->is_dynamic);
    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::var_deref_ref const &inst, builder &b)
  {
    b.next_instruction();
    auto const lifted{ lift_var(inst->qualified_var, b) };
    util::format_to(b.body_buffer, "auto const {}({}->deref());\n", inst->name, lifted);
    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::var_ref_ref const &inst, builder &b)
  {
    b.next_instruction();

    auto const lifted{ lift_var(inst->qualified_var, b) };
    util::format_to(b.body_buffer, "auto const {}({});\n", inst->name, lifted);
    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::type_erase_ref const &inst, builder &b)
  {
    b.next_instruction();

    util::format_to(b.body_buffer,
                    "jank::runtime::object_ref const {}({});\n",
                    inst->name,
                    inst->value);
    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::dynamic_call_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(b.body_buffer, "auto const {}({}.call(", inst->name, inst->fn);
    bool need_comma{};
    for(auto const &arg : inst->args)
    {
      if(need_comma)
      {
        util::format_to(b.body_buffer, ",");
      }
      need_comma = true;
      util::format_to(b.body_buffer, " {}", arg);
    }

    util::format_to(b.body_buffer, "));\n");
    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::literal_ref const &inst, builder &b)
  {
    b.next_instruction();
    auto const lifted{ lift_constant(inst->value, inst->obj, b) };
    util::format_to(b.body_buffer, "auto const {}({});\n", inst->name, lifted);
    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::persistent_list_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(b.body_buffer, "auto const {}(_jank_list(", inst->name);
    if(!is_empty(inst->meta))
    {
      util::format_to(b.body_buffer, "\"{}\", ", util::escape(inst->meta.to_code_string()));
    }
    util::format_to(b.body_buffer, "{}", inst->values.size());
    for(auto const &val : inst->values)
    {
      util::format_to(b.body_buffer, ", {}.erase()", val);
    }
    util::format_to(b.body_buffer, "));\n");

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::persistent_vector_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(b.body_buffer, "auto const {}(_jank_vec(", inst->name);
    if(!is_empty(inst->meta))
    {
      util::format_to(b.body_buffer, "\"{}\", ", util::escape(inst->meta.to_code_string()));
    }
    util::format_to(b.body_buffer, "{}", inst->values.size());
    for(auto const &val : inst->values)
    {
      util::format_to(b.body_buffer, ", {}.erase()", val);
    }
    util::format_to(b.body_buffer, "));\n");

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::persistent_array_map_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(b.body_buffer, "auto const {}(_jank_amap(", inst->name);
    if(!is_empty(inst->meta))
    {
      util::format_to(b.body_buffer, "\"{}\",", util::escape(inst->meta.to_code_string()));
    }

    util::format_to(b.body_buffer, "{}", inst->values.size());
    for(auto const &val : inst->values)
    {
      util::format_to(b.body_buffer, ", {}.erase(), {}.erase()", val.first, val.second);
    }
    util::format_to(b.body_buffer, "));\n");

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::persistent_hash_map_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(b.body_buffer, "auto const {}(_jank_hmap(", inst->name);
    if(!is_empty(inst->meta))
    {
      util::format_to(b.body_buffer, "\"{}\",", util::escape(inst->meta.to_code_string()));
    }

    util::format_to(b.body_buffer, "{}", inst->values.size());
    for(auto const &val : inst->values)
    {
      util::format_to(b.body_buffer, ", {}.erase(), {}.erase()", val.first, val.second);
    }
    util::format_to(b.body_buffer, "));\n");

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::persistent_hash_set_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(b.body_buffer, "auto const {}(_jank_hset(", inst->name);
    if(!is_empty(inst->meta))
    {
      util::format_to(b.body_buffer, "\"{}\",", util::escape(inst->meta.to_code_string()));
    }

    util::format_to(b.body_buffer, "{}", inst->values.size());
    for(auto const &val : inst->values)
    {
      util::format_to(b.body_buffer, ", {}.erase()", val);
    }
    util::format_to(b.body_buffer, "));\n");

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::function_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(b.body_buffer, "auto const {}(", inst->name);
    if(inst->is_variadic)
    {
      util::format_to(b.body_buffer, "_jank_vfn({})", inst->arity_flags);
    }
    else
    {
      util::format_to(b.body_buffer, "_jank_fn({})", inst->arity_flags);
    }
    util::format_to(b.body_buffer, ");\n");

    for(auto const &arity : inst->arities)
    {
      util::format_to(b.body_buffer,
                      "{}->arity_{} = &{};\n",
                      inst->name,
                      arity.first,
                      arity.second);
      builder nested{ b.module, arity.second };
      gen(*nested.function, nested);
      util::format_to(b.deps_buffer, "{}", nested.declaration_str());
    }

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::closure_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(b.deps_buffer, "struct {}{", inst->context);
    util::format_to(b.body_buffer,
                    "auto const {}(jtl::make_ref<struct {}>(",
                    inst->context,
                    inst->context);

    bool need_comma{};
    for(auto const &capture : inst->captures)
    {
      util::format_to(b.deps_buffer,
                      "{} {};\n",
                      get_qualified_type_name(capture.second.type),
                      munge(capture.first));

      if(need_comma)
      {
        util::format_to(b.body_buffer, ", ");
      }
      need_comma = true;
      if(capture.second.name == ":defer")
      {
        b.defer(inst->context, capture.first);
        b.body_buffer("jank::runtime::jank_nil");
      }
      else
      {
        b.body_buffer(capture.second.name);
      }
    }

    util::format_to(b.deps_buffer, "};\n");
    util::format_to(b.body_buffer, "));\n");


    util::format_to(b.body_buffer, "auto const {}(", inst->name);
    if(inst->is_variadic)
    {
      util::format_to(b.body_buffer,
                      "_jank_vclosure({}, {}.data)",
                      inst->arity_flags,
                      inst->context);
    }
    else
    {
      util::format_to(b.body_buffer,
                      "_jank_closure({}, {}.data)",
                      inst->arity_flags,
                      inst->context);
    }
    util::format_to(b.body_buffer, ");\n");

    for(auto const &arity : inst->arities)
    {
      util::format_to(b.body_buffer,
                      "{}->arity_{} = &{};\n",
                      inst->name,
                      arity.first,
                      arity.second);
      builder nested{ b.module, arity.second };
      gen(*nested.function, nested);
      util::format_to(b.deps_buffer, "{}", nested.declaration_str());
    }

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::nop_ref const &, builder &b)
  {
    b.next_instruction();
    return none;
  }

  jtl::option<identifier> gen(ir::inst::parameter_ref const &inst, builder &b)
  {
    b.next_instruction();
    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::capture_ref const &inst, builder &b)
  {
    b.next_instruction();
    auto const &closure_ctx{ munge(b.function->arity->fn_ctx->fn->unique_name + "_ctx") };
    util::format_to(b.body_buffer,
                    "auto &&{}({}->{});\n",
                    inst->name,
                    closure_ctx,
                    munge(inst->value));
    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::recursion_reference_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(b.body_buffer,
                    "auto &&{}({});\n",
                    inst->name,
                    munge(b.function->arity->fn_ctx->fn->name));
    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::named_recursion_ref const &inst, builder &b)
  {
    b.next_instruction();

    bool needs_comma{};
    if(inst->needs_dynamic_call)
    {
      util::format_to(b.body_buffer, "auto const {}({}.call(", inst->name, inst->fn);
    }
    else
    {
      auto const fn_name{ util::format("{}_{}", inst->fn_base_name, inst->args.size()) };
      util::format_to(b.body_buffer, "auto const {}({}({}", inst->name, fn_name, inst->fn);
      needs_comma = true;

      /* TODO: Save some state that we did this so we don't do it again. */
      //if(b.function->arity->params.size() < inst->args.size())
      {
        util::format_to(b.deps_buffer,
                        "extern \"C\" jank::runtime::object_ref {}(jank::runtime::object_ref const",
                        fn_name);

        for(auto const &_ : inst->args)
        {
          util::format_to(b.deps_buffer, ", jank::runtime::object_ref");
        }

        util::format_to(b.deps_buffer, ");\n");
      }
    }

    for(auto const &arg : inst->args)
    {
      if(needs_comma)
      {
        util::format_to(b.body_buffer, ",");
      }
      needs_comma = true;
      util::format_to(b.body_buffer, " {}", arg);
    }

    util::format_to(b.body_buffer, "));\n");
    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::letfn_ref const &inst, builder &b)
  {
    b.next_instruction();

    native_unordered_map<jtl::immutable_string, ir::identifier> bindings;
    for(auto const &bind : inst->bindings)
    {
      bindings[bind]
        = gen(b.function->blocks[b.block_index].instructions[b.instruction_index], b).unwrap();
    }

    for(auto const &deferred : b.deferred_bindings)
    {
      util::format_to(b.body_buffer,
                      "{}->{} = {};\n",
                      deferred.first,
                      deferred.second,
                      bindings[deferred.second]);
    }

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::jump_ref const &inst, builder &b)
  {
    b.next_instruction();

    if(inst->loop)
    {
      util::format_to(b.body_buffer, "continue;\n");
    }

    if(b.seen_blocks.contains(inst->block))
    {
      return none;
    }
    b.enter_block(inst->block);
    return none;
  }

  jtl::option<identifier> gen(ir::inst::truthy_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(b.body_buffer,
                    "auto const {}(jank::runtime::truthy({}));\n",
                    inst->name,
                    inst->value);
    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::branch_get_ref const &inst, builder &b)
  {
    b.next_instruction();
    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::branch_set_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(b.body_buffer, "{} = {};\n", inst->shadow, inst->value);
    return none;
  }

  jtl::option<identifier> gen(ir::inst::branch_ref const &inst, builder &b)
  {
    if(inst->shadow.is_some())
    {
      util::format_to(b.body_buffer,
                      "{} {}{ };\n",
                      get_qualified_type_name(inst->shadow.unwrap().type),
                      inst->shadow.unwrap().name);
    }

    util::format_to(b.body_buffer, "if({}){\n", inst->condition);
    b.enter_block(inst->then_block);
    gen_until_jump(inst->merge_block, b);

    util::format_to(b.body_buffer, "} else {\n");
    b.enter_block(inst->else_block);
    gen_until_jump(inst->merge_block, b);
    util::format_to(b.body_buffer, "}\n");

    if(inst->merge_block.is_some())
    {
      b.enter_block(inst->merge_block.unwrap());
    }

    return none;
  }

  jtl::option<identifier> gen(ir::inst::loop_ref const &inst, builder &b)
  {
    b.next_instruction();

    if(inst->shadow.is_some())
    {
      util::format_to(b.body_buffer,
                      "{} {};\n",
                      get_qualified_type_name(inst->shadow.unwrap().type),
                      inst->shadow.unwrap().name);
    }

    for(auto const &shadow : inst->binding_shadows)
    {
      if(is_any_object(shadow.type))
      {
        util::format_to(b.body_buffer,
                        "jank::runtime::object_ref {}({});\n",
                        shadow.name,
                        shadow.value);
      }
      else
      {
        util::format_to(b.body_buffer, "auto {}({});\n", shadow.name, shadow.value);
      }
    }

    util::format_to(b.body_buffer, "while(true){\n");
    b.enter_block(inst->loop_block);
    gen_until_jump(inst->merge_block, b);
    util::format_to(b.body_buffer, " break; }\n");

    if(inst->merge_block.is_some())
    {
      b.enter_block(inst->merge_block.unwrap());
    }

    return none;
  }

  jtl::option<identifier> gen(ir::inst::throw_ref const &inst, builder &b)
  {
    b.next_instruction();
    /* We static_cast to object_ref here, since we'll be trying to catch an object_ref in any
     * try/catch forms. This loses us our type info, but C++ doesn't do implicit conversions
     * when catching and we're not using inheritance. */
    util::format_to(b.body_buffer,
                    "throw static_cast<jank::runtime::object_ref>({});\n",
                    inst->value);
    return none;
  }

  jtl::option<identifier> gen(ir::inst::try_ref const &inst, builder &b)
  {
    b.next_instruction();
    auto const has_finally{ inst->finally_block.is_some() };
    identifier finally_guard_name;
    util::format_to(b.body_buffer, "jank::runtime::object_ref {};\n", inst->shadow);

    if(has_finally)
    {
      util::format_to(b.body_buffer, "{");
      auto const instruction_index{ b.instruction_index };
      auto const block_index{ b.block_index };
      b.enter_block(inst->finally_block.unwrap());

      finally_guard_name
        = b.function->blocks[b.block_index].instructions[b.instruction_index]->name;

      gen(b.function->blocks[b.block_index].instructions[b.instruction_index], b);

      b.instruction_index = instruction_index;
      b.block_index = block_index;
    }

    util::format_to(b.body_buffer, "try {\n");

    auto const &jump_block{ has_finally ? inst->finally_block : inst->merge_block };

    gen_until_jump(jump_block, b);

    util::format_to(b.body_buffer, "}\n");
    for(auto const &catch_details : inst->catches)
    {
      b.enter_block(catch_details.second);
      gen(b.function->blocks[b.block_index].instructions[b.instruction_index], b);
    }

    if(has_finally)
    {
      auto const finally_name{ util::format("{}_fn", finally_guard_name) };
      util::format_to(b.body_buffer,
                      "catch(...) { {}.release(); {}(); throw; } {}.release(); {}(); }\n",
                      finally_guard_name,
                      finally_name,
                      finally_guard_name,
                      finally_name);
    }

    b.enter_block(inst->merge_block);

    return none;
  }

  jtl::option<identifier> gen(ir::inst::catch_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(b.body_buffer,
                    "catch({} {}) {",
                    get_qualified_type_name(inst->type),
                    inst->name);

    auto const &jump_block{ inst->finally_block.is_some() ? inst->finally_block
                                                          : inst->merge_block };
    gen_until_jump(jump_block, b);
    util::format_to(b.body_buffer, "}\n");
    return none;
  }

  jtl::option<identifier> gen(ir::inst::finally_ref const &inst, builder &b)
  {
    b.next_instruction();

    auto const fn_name{ inst->name + "_fn" };
    util::format_to(b.body_buffer, "auto const {}{ [&](){\n", fn_name);
    gen_until_jump(inst->merge_block, b);
    util::format_to(b.body_buffer, "} };\n");
    util::format_to(b.body_buffer, "jank::util::scope_exit {}{ {}, true };\n", inst->name, fn_name);
    return none;
  }

  jtl::option<identifier> gen(ir::inst::case_ref const &inst, builder &b)
  {
    b.next_instruction();

    if(inst->shadow.is_some())
    {
      util::format_to(b.body_buffer, "jank::runtime::object_ref {};\n", inst->shadow.unwrap());
    }

    util::format_to(b.body_buffer,
                    "switch(jank_shift_mask_case_integer(static_cast<jank::runtime::object*>({}."
                    "erase().raw()), {}, {})) {\n",
                    inst->value,
                    inst->shift,
                    inst->mask);

    for(auto const &case_block : inst->case_blocks)
    {
      util::format_to(b.body_buffer, "case {}: {\n", case_block.first);
      b.enter_block(case_block.second);
      gen_until_jump(inst->merge_block, b);
      util::format_to(b.body_buffer, "break; }");
    }

    util::format_to(b.body_buffer, "default: {\n");
    b.enter_block(inst->default_block);
    gen_until_jump(inst->merge_block, b);
    util::format_to(b.body_buffer, "break; }\n");

    util::format_to(b.body_buffer, "}\n");

    if(inst->merge_block.is_some())
    {
      b.enter_block(inst->merge_block.unwrap());
    }

    return none;
  }

  jtl::option<identifier> gen(ir::inst::ret_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(b.body_buffer, "return {};\n", inst->value);

    return none;
  }

  jtl::option<identifier> gen(ir::inst::cpp_raw_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(b.cpp_raw_buffer, "\n{}\n", inst->expr->code);

    return none;
  }

  jtl::option<identifier> gen(ir::inst::cpp_value_ref const &inst, builder &b)
  {
    b.next_instruction();

    if(inst->expr->val_kind == analyze::expr::cpp_value::value_kind::null)
    {
      util::format_to(b.body_buffer, "auto &&{}(nullptr);\n", inst->name);
    }
    else if(inst->expr->val_kind == analyze::expr::cpp_value::value_kind::bool_true
            || inst->expr->val_kind == analyze::expr::cpp_value::value_kind::bool_false)
    {
      auto const val{ inst->expr->val_kind == analyze::expr::cpp_value::value_kind::bool_true };
      util::format_to(b.body_buffer, "auto &&{}({});\n", inst->name, val);
    }
    /* Static const primitives need to be copied, since they won't have linkage. */
    else if(Cpp::IsStaticVariable(inst->expr->scope)
            && Cpp::IsConstType(Cpp::GetNonReferenceType(inst->expr->type))
            && is_primitive(Cpp::GetNonReferenceType(inst->expr->type)))
    {
      util::format_to(b.body_buffer,
                      "auto {}({});\n",
                      inst->name,
                      Cpp::GetQualifiedCompleteNameWithTemplateArgs(inst->expr->scope));
    }
    /* Functions referred to by value should get a cast, in case they're overloaded. */
    else if(Cpp::IsFunction(inst->expr->scope) || Cpp::IsTemplatedFunction(inst->expr->scope))
    {
      util::format_to(b.body_buffer,
                      "auto &&{}(static_cast<{}>(&{}));\n",
                      inst->name,
                      get_qualified_type_name(inst->expr->type),
                      Cpp::GetQualifiedCompleteNameWithTemplateArgs(inst->expr->scope));
    }
    else if(Cpp::IsArrayType(Cpp::GetNonReferenceType(inst->expr->type)))
    {
      util::format_to(b.body_buffer,
                      "{} {}({});\n",
                      get_qualified_type_name(Cpp::GetPointerType(
                        Cpp::GetArrayElementType(Cpp::GetNonReferenceType(inst->expr->type)))),
                      inst->name,
                      Cpp::GetQualifiedCompleteNameWithTemplateArgs(inst->expr->scope));
    }
    else
    {
      util::format_to(b.body_buffer,
                      "auto &&{}({}{});\n",
                      inst->name,
                      (Cpp::IsPointerToMemberType(inst->expr->type) ? "&" : ""),
                      Cpp::GetQualifiedCompleteNameWithTemplateArgs(inst->expr->scope));
    }

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::cpp_into_object_ref const &inst, builder &b)
  {
    b.next_instruction();
    /* There's no need to do a conversion for void, since we always just
     * want nil. There's no need for generating a tmp for it either, since
     * we have a global nil constant. */
    if(Cpp::IsVoid(inst->expr->conversion_type))
    {
      util::format_to(b.body_buffer, "auto const {}(jank::runtime::jank_nil);\n", inst->name);
      return inst->name;
    }

    /* We can rely on the C++ type system to handle conversion from typed objects
     * to untype objects. */
    if(is_untyped_object(inst->expr->type) && is_any_object(inst->expr->conversion_type))
    {
      util::format_to(b.body_buffer, "auto &&{}({});\n", inst->name, inst->value);
      return inst->name;
    }

    util::format_to(b.body_buffer,
                    "auto const {}(jank::runtime::convert<{}>::{}({}));\n",
                    inst->name,
                    get_qualified_type_name(Cpp::GetCanonicalType(Cpp::GetTypeWithoutCv(
                      Cpp::GetNonReferenceType(inst->expr->conversion_type)))),
                    (inst->expr->policy == analyze::conversion_policy::into_object ? "into_object"
                                                                                   : "from_object"),
                    inst->value);

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::cpp_from_object_ref const &inst, builder &b)
  {
    ir::inst::cpp_into_object from{ inst->name, inst->location, inst->value, inst->expr };
    return gen(ir::inst::cpp_into_object_ref{ &from }, b);
  }

  jtl::option<identifier> gen(ir::inst::cpp_unsafe_cast_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(b.body_buffer,
                    "auto const {}(({})({}));\n",
                    inst->name,
                    get_qualified_type_name(inst->expr->type),
                    inst->value);

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::cpp_call_ref const &inst, builder &b)
  {
    b.next_instruction();
    if((b.module->target == compilation_target::module
        || b.module->target == compilation_target::module_function)
       && !inst->expr->function_code.empty())
    {
      util::format_to(b.cpp_raw_buffer, "\n{}\n", inst->expr->function_code);
    }

    auto const source_type{ expression_type(inst->expr->source_expr) };

    if(inst->expr->source_expr->kind == analyze::expression_kind::cpp_value)
    {
      auto const source{ static_cast<analyze::expr::cpp_value *>(inst->expr->source_expr.data) };

      auto const is_void{ Cpp::IsVoid(Cpp::GetFunctionReturnType(source->scope)) };
      if(is_void)
      {
        util::format_to(b.body_buffer, "jank::runtime::object_ref {};\n", inst->name);
      }
      else
      {
        util::format_to(b.body_buffer, "auto &&{}(", inst->name);
      }

      util::format_to(b.body_buffer, "{}(", Cpp::GetQualifiedCompleteName(source->scope));

      bool need_comma{};
      for(usize arg_idx{}; arg_idx < inst->expr->arg_exprs.size(); ++arg_idx)
      {
        auto const arg_expr{ inst->expr->arg_exprs[arg_idx] };
        auto const arg_type{ expression_type(arg_expr) };
        /* This will be null in variadic positions. */
        auto const param_type{ Cpp::GetFunctionArgType(source->scope, arg_idx) };
        auto const &arg_tmp{ inst->args[arg_idx] };

        if(need_comma)
        {
          util::format_to(b.body_buffer, ", ");
        }

        if(param_type && Cpp::IsPointerType(param_type) && is_any_object(arg_type))
        {
          util::format_to(b.body_buffer, "static_cast<jank::runtime::object*>(");
        }

        if(param_type && Cpp::IsRvalueReferenceType(param_type))
        {
          util::format_to(b.body_buffer, "std::move({})", arg_tmp);
        }
        else
        {
          util::format_to(b.body_buffer, "{}", arg_tmp);
        }

        if(param_type && Cpp::IsPointerType(param_type) && is_any_object(arg_type))
        {
          util::format_to(b.body_buffer, ".erase().raw())");
        }
        need_comma = true;
      }

      util::format_to(b.body_buffer, ")");

      if(!is_void)
      {
        util::format_to(b.body_buffer, ");\n");
      }
      else
      {
        util::format_to(b.body_buffer, ";\n");
      }

      return inst->name;
    }
    else if(Cpp::IsPointerToMemberVariableType(source_type))
    {
      auto const is_void{ Cpp::IsVoid(inst->expr->type) };
      if(is_void)
      {
        util::format_to(b.body_buffer, "jank::runtime::object_ref const {};\n", inst->name);
      }
      else
      {
        util::format_to(b.body_buffer, "auto &&{}(", inst->name);
      }

      auto const obj_type{ Cpp::GetNonReferenceType(expression_type(inst->expr->arg_exprs[0])) };
      auto const &obj_name{ inst->args[0] };
      if(Cpp::IsPointerType(obj_type))
      {
        util::format_to(b.body_buffer, "{}->*{}", obj_name, inst->value.unwrap());
      }
      else
      {
        util::format_to(b.body_buffer, "{}.*{}", obj_name, inst->value.unwrap());
      }

      if(!is_void)
      {
        util::format_to(b.body_buffer, ");\n");
      }
      else
      {
        util::format_to(b.body_buffer, ";\n");
      }

      return inst->name;
    }
    else if(Cpp::IsPointerToMemberFunctionType(source_type))
    {
      auto const is_void{ Cpp::IsVoid(inst->expr->type) };
      if(is_void)
      {
        util::format_to(b.body_buffer, "jank::runtime::object_ref const {};\n", inst->name);
      }
      else
      {
        util::format_to(b.body_buffer, "auto &&{}(", inst->name);
      }

      auto const obj_type{ Cpp::GetNonReferenceType(expression_type(inst->expr->arg_exprs[0])) };
      auto const &obj_name{ inst->args[0] };
      if(Cpp::IsPointerType(obj_type))
      {
        util::format_to(b.body_buffer, "({}->*{})(", obj_name, inst->value.unwrap());
      }
      else
      {
        util::format_to(b.body_buffer, "({}.*{})(", obj_name, inst->value.unwrap());
      }

      bool need_comma{};
      for(auto it{ inst->args.begin() + 1 }; it != inst->args.end(); ++it)
      {
        if(need_comma)
        {
          util::format_to(b.body_buffer, ", ");
        }
        util::format_to(b.body_buffer, "{}", *it);
        need_comma = true;
      }

      util::format_to(b.body_buffer, ")");

      if(!is_void)
      {
        util::format_to(b.body_buffer, ");\n");
      }
      else
      {
        util::format_to(b.body_buffer, ";\n");
      }

      return inst->name;
    }
    else
    {
      auto const is_void{ Cpp::IsVoid(inst->expr->type) };
      if(is_void)
      {
        util::format_to(b.body_buffer, "jank::runtime::object_ref const {};", inst->name);
      }
      else
      {
        util::format_to(b.body_buffer, "auto &&{}(", inst->name);
      }

      util::format_to(b.body_buffer, "{}(", inst->value.unwrap());

      bool need_comma{};
      for(auto const &arg : inst->args)
      {
        if(need_comma)
        {
          util::format_to(b.body_buffer, ", ");
        }
        util::format_to(b.body_buffer, "{}", arg);
        need_comma = true;
      }

      util::format_to(b.body_buffer, ")");

      if(!is_void)
      {
        util::format_to(b.body_buffer, ");\n");
      }
      else
      {
        util::format_to(b.body_buffer, ";\n");
      }

      return inst->name;
    }
  }

  jtl::option<identifier> gen(ir::inst::cpp_constructor_call_ref const &inst, builder &b)
  {
    b.next_instruction();
    auto const non_ref_type{ Cpp::GetNonReferenceType(inst->expr->type) };

    if(inst->args.empty())
    {
      if(Cpp::IsFunctionPointerType(inst->expr->type))
      {
        util::format_to(
          b.body_buffer,
          "{} ",
          get_qualified_type_name(Cpp::GetFunctionReturnTypeFromType(inst->expr->type)));
        util::format_to(b.body_buffer,
                        "(* {} {} {})(",
                        Cpp::IsConstType(inst->expr->type) ? "const" : "",
                        Cpp::HasTypeQualifier(inst->expr->type, Cpp::Volatile) ? "volatile" : "",
                        inst->name);
        auto const param_count{ Cpp::GetFunctionNumArgsFromType(inst->expr->type) };
        for(usize i{}; i < param_count; ++i)
        {
          auto const param_type{ Cpp::GetFunctionArgTypeFromType(inst->expr->type, i) };
          util::format_to(b.body_buffer,
                          "{} {}",
                          (i != 0) ? ", " : "",
                          get_qualified_type_name(param_type));
        }
        util::format_to(b.body_buffer, "){ };\n");
      }
      else if(Cpp::IsArrayType(non_ref_type)
              || (Cpp::IsPointerType(non_ref_type)
                  && Cpp::IsArrayType(Cpp::GetUnderlyingType(non_ref_type))))
      {
        auto const array_type{ Cpp::IsPointerType(non_ref_type)
                                 ? Cpp::GetUnderlyingType(non_ref_type)
                                 : non_ref_type };
        util::format_to(
          b.body_buffer,
          "{} ({}{})[{}]{ };\n",
          get_qualified_type_name(Cpp::GetArrayElementType(array_type)),
          (Cpp::IsPointerType(inst->expr->type)
             ? "*"
             /* NOLINTNEXTLINE(readability-avoid-nested-conditional-operator) */
             : (Cpp::IsReferenceType(inst->expr->type) ? "&" : "")),
          inst->name,
          Cpp::IsSizedArrayType(array_type) ? std::to_string(Cpp::GetArraySize(array_type)) : "");
      }
      else
      {
        util::format_to(b.body_buffer,
                        "{} {}{ };\n",
                        get_qualified_type_name(inst->expr->type),
                        inst->name);
      }
      return inst->name;
    }

    native_vector<void *> param_types;
    if(inst->expr->fn)
    {
      auto const param_count{ Cpp::GetFunctionNumArgs(inst->expr->fn) };
      for(usize i{}; i < param_count; ++i)
      {
        param_types.emplace_back(Cpp::GetFunctionArgType(inst->expr->fn, i));
      }
    }
    else if(is_primitive(Cpp::GetNonReferenceType(inst->expr->type)))
    {
      param_types.emplace_back(inst->expr->type);
    }
    else
    {
      jank_debug_assert(inst->expr->is_aggregate);
      auto const scope{ Cpp::GetScopeFromType(inst->expr->type) };
      jank_debug_assert(scope);
      auto const member_types{ aggregate_initialization_types(scope) };
      std::ranges::copy(member_types, std::back_inserter(param_types));
    }
    jank_debug_assert(inst->expr->arg_exprs.size() <= param_types.size());

    if(Cpp::IsArrayType(Cpp::GetNonReferenceType(inst->expr->type)))
    {
      util::format_to(b.body_buffer, "auto {} ", inst->name);
    }
    else
    {
      util::format_to(b.body_buffer,
                      "{} {} ",
                      get_qualified_type_name(inst->expr->type),
                      inst->name);
    }

    /* For aggregate initialization, we want to use the uniform initialization syntax. However,
     * for any other initialization, we're expecting to call a ctor, so we use parens. This
     * removes any ambiguity when there is a ctor which takes an initializer list, which we
     * don't currently support. */
    util::format_to(b.body_buffer, "{}", (inst->expr->is_aggregate ? "{" : "("));

    bool need_comma{};
    for(usize arg_idx{}; arg_idx < inst->expr->arg_exprs.size(); ++arg_idx)
    {
      if(need_comma)
      {
        util::format_to(b.body_buffer, ", ");
      }
      need_comma = true;

      auto const arg_type{ expression_type(inst->expr->arg_exprs[arg_idx]) };
      bool needs_conversion{};
      jtl::immutable_string conversion_direction, trait_type;
      if(is_any_object(param_types[arg_idx]) && !is_any_object(arg_type))
      {
        needs_conversion = true;
        conversion_direction = "into_object";
        trait_type = get_qualified_type_name(arg_type);
      }
      else if(!is_any_object(param_types[arg_idx]) && is_any_object(arg_type))
      {
        needs_conversion = true;
        conversion_direction = "from_object";
        trait_type = get_qualified_type_name(param_types[arg_idx]);
      }

      if(needs_conversion)
      {
        util::format_to(b.body_buffer,
                        "jank::runtime::convert<{}>::{}({})",
                        trait_type,
                        conversion_direction,
                        inst->args[arg_idx]);
      }
      else
      {
        auto const needs_static_cast{ param_types[arg_idx] != arg_type };
        if(needs_static_cast)
        {
          util::format_to(b.body_buffer,
                          "static_cast<{}>(",
                          get_qualified_type_name(param_types[arg_idx]));
        }

        util::format_to(b.body_buffer, "{}", inst->args[arg_idx]);

        if(needs_static_cast)
        {
          util::format_to(b.body_buffer, ")");
        }
      }
    }

    util::format_to(b.body_buffer, "{};\n", (inst->expr->is_aggregate ? "}" : ")"));

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::cpp_member_call_ref const &inst, builder &b)
  {
    b.next_instruction();
    auto const fn_name{ Cpp::GetName(inst->expr->fn) };
    auto const is_void{ Cpp::IsVoid(Cpp::GetFunctionReturnType(inst->expr->fn)) };

    if(is_void)
    {
      util::format_to(b.body_buffer, "jank::runtime::object_ref {}{ };\n", inst->name);
      util::format_to(b.body_buffer,
                      "{}{}{}(",
                      inst->args[0],
                      (Cpp::IsPointerType(expression_type(inst->expr->arg_exprs[0])) ? "->" : "."),
                      fn_name);
    }
    else
    {
      util::format_to(b.body_buffer,
                      "auto &&{}({}{}{}(",
                      inst->name,
                      inst->args[0],
                      (Cpp::IsPointerType(expression_type(inst->expr->arg_exprs[0])) ? "->" : "."),
                      fn_name);
    }

    bool need_comma{};
    for(auto it{ inst->args.begin() + 1 }; it != inst->args.end(); ++it)
    {
      if(need_comma)
      {
        util::format_to(b.body_buffer, ", ");
      }
      util::format_to(b.body_buffer, "{}", *it);
      need_comma = true;
    }

    if(is_void)
    {
      util::format_to(b.body_buffer, ");\n");
    }
    else
    {
      util::format_to(b.body_buffer, "));\n");
    }

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::cpp_member_access_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(
      b.body_buffer,
      "auto &&{}({}{}{});\n",
      inst->name,
      inst->value,
      (Cpp::IsPointerType(Cpp::GetNonReferenceType(expression_type(inst->expr->obj_expr))) ? "->"
                                                                                           : "."),
      inst->expr->name);

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::cpp_builtin_operator_call_ref const &inst, builder &b)
  {
    b.next_instruction();
    auto const op_name{ operator_name(static_cast<Cpp::Operator>(inst->expr->op)).unwrap() };

    if(inst->args.size() == 1)
    {
      util::format_to(b.body_buffer, "auto &&{}( {}{} );\n", inst->name, op_name, inst->args[0]);
    }
    else if(op_name == "aget")
    {
      util::format_to(b.body_buffer,
                      "auto &&{}( {}[{}] );\n",
                      inst->name,
                      inst->args[0],
                      inst->args[1]);
    }
    else
    {
      util::format_to(b.body_buffer,
                      "auto &&{}( {} {} {} );\n",
                      inst->name,
                      inst->args[0],
                      op_name,
                      inst->args[1]);
    }

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::cpp_box_ref const &inst, builder &b)
  {
    b.next_instruction();
    auto const value_expr_type{ expression_type(inst->expr->value_expr) };
    auto const type_str{ get_qualified_type_name(
      Cpp::GetCanonicalType(Cpp::GetNonReferenceType(value_expr_type))) };

    util::format_to(
      b.body_buffer,
      "auto {}{ jank::runtime::make_box<jank::runtime::obj::opaque_box>({}, \"{}\") };\n",
      inst->name,
      inst->value,
      type_str);

    auto const meta{ source_to_meta(inst->expr->source) };
    util::format_to(b.body_buffer,
                    /* TODO: Lift this. */
                    "jank::runtime::reset_meta({}, _jank_eval_str(\"{}\"));\n",
                    inst->name,
                    util::escape(to_code_string(meta)));

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::cpp_unbox_ref const &inst, builder &b)
  {
    b.next_instruction();
    auto const type_name{ get_qualified_type_name(Cpp::GetCanonicalType(inst->expr->type)) };
    util::format_to(
      b.body_buffer,
      "auto {}{ "
      "static_cast<{}>(jank_unbox_with_source(\"{}\", {}.erase().raw(), {}.erase().raw())) };\n",
      inst->name,
      type_name,
      type_name,
      inst->value,
      inst->meta);

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::cpp_new_ref const &inst, builder &b)
  {
    b.next_instruction();
    auto finalizer_name{ munge(__rt_ctx->unique_string("finalizer")) };
    auto const type_name{ get_qualified_type_name(inst->expr->type) };
    auto const needs_finalizer{ !Cpp::IsTriviallyDestructible(inst->expr->type) };

    if(needs_finalizer)
    {
      util::format_to(b.body_buffer,
                      "static auto const {}("
                      "[](void * const obj, void *){"
                      "using T = {};"
                      "reinterpret_cast<T*>(obj)->~T();"
                      "});\n",
                      finalizer_name,
                      type_name);
    }

    util::format_to(b.body_buffer,
                    "auto {}{ "
                    "new (UseGC{}) {}{ {} }"
                    " };\n",
                    inst->name,
                    (needs_finalizer ? ", " + finalizer_name : ""),
                    type_name,
                    inst->value);

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::cpp_def_ref const &inst, builder &b)
  {
    b.next_instruction();
    auto const type_name{ get_qualified_type_name(inst->expr->type) };
    auto const munged_current_ns{ runtime::munge(__rt_ctx->current_ns()->name->name) };
    auto const munged_var_name{ runtime::munge(inst->expr->name->get_name()) };

    auto const native_ns{ module::module_to_native_ns(munged_current_ns) };

    util::format_to(b.cpp_def_buffer, "{} {}{ };", type_name, munged_var_name);
    if(inst->value.is_some())
    {
      util::format_to(b.body_buffer,
                      "{}::{} = {};",
                      native_ns,
                      munged_var_name,
                      inst->value.unwrap());
    }

    util::format_to(b.body_buffer,
                    R"(_jank_refer_global("{}.{}", "{}");)",
                    munged_current_ns,
                    munged_var_name,
                    inst->expr->name->name);

    util::format_to(b.body_buffer, "auto {}{jank::runtime::jank_nil};", inst->name);

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::cpp_delete_ref const &inst, builder &b)
  {
    b.next_instruction();
    auto const value_type{ Cpp::GetPointeeType(expression_type(inst->expr->value_expr)) };
    auto const type_name{ get_qualified_type_name(value_type) };
    auto const needs_finalizer{ !Cpp::IsTriviallyDestructible(value_type) };

    /* Calling GC_free won't trigger the finalizer. Not sure why, but it's explicitly
     * documented in bdwgc. So, we'll invoke it manually if needed, prior to GC_free. */
    if(needs_finalizer)
    {
      util::format_to(b.body_buffer,
                      "{ using T = {};\n"
                      "{}->~T(); }\n",
                      type_name,
                      inst->value);
    }

    util::format_to(b.body_buffer, "GC_free({});\n", inst->value);

    return "jank::runtime::jank_nil";
  }

  jtl::option<identifier> gen(ir::instruction_ref const &inst, builder &b)
  {
    jtl::string_builder sb;
    inst->print(sb, 0);
    //util::println("gen {}", sb.release());
    if(util::cli::opts.debug)
    {
      auto const &location{ inst->location };

      if(location != read::source::unknown())
      {
        util::format_to(b.body_buffer,
                        "\n#line {} \"{}\"\n",
                        location.start.line,
                        util::escape(location.file));
      }
    }
    jtl::option<identifier> name;
    ir::visit_inst([&](auto const typed_inst) { name = gen(typed_inst, b); }, inst);
    return name;
  }

  void gen(ir::function const &fn, builder &b)
  {
    auto const &all_captures{ fn.arity->frame->captures };
    auto const &munged_fn_name{ munge(fn.arity->fn_ctx->fn->name) };
    auto const &munged_linkage_name{ munge(fn.arity->fn_ctx->fn->unique_name) };
    auto const &closure_ctx{ munge(fn.arity->fn_ctx->fn->unique_name + "_ctx") };

    bool param_shadows_fn{};
    for(auto const &param : fn.arity->params)
    {
      param_shadows_fn |= param->name == fn.arity->fn_ctx->fn->name;
    }

    util::format_to(
      b.body_buffer,
      "\nextern \"C\" jank::runtime::object_ref {}_{}(jank::runtime::object_ref const {}",
      munged_linkage_name,
      fn.arity->params.size(),
      param_shadows_fn ? "" : munged_fn_name);

    for(auto const &param : fn.arity->params)
    {
      util::format_to(b.body_buffer, ", jank::runtime::object_ref {}", munge(param->name));
    }

    util::format_to(b.body_buffer, ") {\n");

    //util::format_to(body_buffer, "jank::profile::timer __timer{ \"{}\" };", root_fn->name);

    if(!all_captures.empty())
    {
      util::format_to(b.body_buffer,
                      "auto const * const {}{ "
                      "static_cast<struct {}*>(static_cast<jank::runtime::obj::jit_"
                      "closure*>({}.ptr())->context) };\n",
                      closure_ctx,
                      closure_ctx,
                      munged_fn_name);
    }

    b.block_index = 0;
    b.instruction_index = 0;
    while(b.instruction_index < b.function->blocks[b.block_index].instructions.size())
    {
      gen(b.function->blocks[b.block_index].instructions[b.instruction_index], b);
    }

    if(fn.arity->body->values.empty())
    {
      util::format_to(b.body_buffer, "return { };\n");
    }

    util::format_to(b.body_buffer, "}\n");
  }

  generated_cpp gen_cpp(ir::module const &mod)
  {
    builder b{ &mod, mod.entry_points[0] };

    for(auto const &fn_name : mod.entry_points)
    {
      auto const fn{ find_function(&mod, fn_name) };
      b.function = fn;
      gen(*fn, b);
      b.seen_blocks.clear();
    }

    /* Module targeting works in a special way, with the goal of
     * cutting down the generated code size. Instead of each function
     * having its own lifted vars/constants, we have one namespace for
     * the module with the lifted globals there, at namespace level.
     * Then every function within that module can share the same globals.
     * This also makes creating functions cheaper. However, it requires
     * some special tracking. */
    if(mod.target == compilation_target::module)
    {
      util::format_to(b.module_header_buffer,
                      "namespace {} {\n",
                      module::module_to_native_ns(mod.name));

      util::format_to(b.module_header_buffer, "{}", b.cpp_def_buffer.view());

      /* We need to initialize these with the special _jank_null, which temporarily stores
       * a nullptr within them. This isn't normally allowed, but we can't assume we have
       * access to jank_nil when these are initialized because initialization order across
       * C++ translation units is undefined. */
      for(auto const &v : b.module->lifted_constants)
      {
        util::format_to(b.module_header_buffer,
                        "{} {}{ _jank_null{ } };\n",
                        get_qualified_type_name(literal_codegen_type(v.first)),
                        v.second);
      }
      for(auto const &v : b.module->lifted_vars)
      {
        util::format_to(b.module_header_buffer,
                        "jank::runtime::var_ref {}{ _jank_null{ } };\n",
                        v.second.name);
      }
    }
    else if(!b.cpp_def_buffer.empty())
    {
      util::format_to(b.module_header_buffer,
                      "namespace {}{ {} }",
                      module::module_to_native_ns(__rt_ctx->current_ns()->name->name),
                      b.cpp_def_buffer.view());
    }

    if(mod.target == compilation_target::module)
    {
      util::format_to(b.footer_buffer,
                      "extern \"C\" void {}(){",
                      module::module_to_load_function(mod.name));

      /* First thing we do when loading this module is to intern our ns. Everything else will
       * build on that. */
      util::format_to(b.footer_buffer, "jank_ns_intern_c(\"{}\");", mod.name);

      /* This dance is performed to keep symbol names unique across all the modules.
       * Considering LLVM JIT symbols to be global, we need to define them with
       * unique names to avoid conflicts during JIT recompilation/reloading.
       *
       * The approach, right now, is for each namespace, we will keep a counter
       * and will increase it every time we define a new symbol. When we JIT reload
       * the same namespace again, we will define new symbols.
       *
       * This IR codegen for calling `jank_ns_set_symbol_counter`, is to set the counter
       * on an initial load.
       */
      auto const current_ns{ __rt_ctx->current_ns() };
      util::format_to(b.footer_buffer,
                      "jank_ns_set_symbol_counter(\"{}\", {});\n",
                      current_ns->name->get_name(),
                      current_ns->symbol_counter.load());

      auto const native_ns{ module::module_to_native_ns(mod.name) };

      /* BDWGC doesn't pick up globals in JIT compiled code, so we need to register both
       * our lifted vars and lifted constants. Since they're right next to each other,
       * we can just register the range of the first -> last. */
      if(!b.module->lifted_vars.empty())
      {
        auto const &first{ *b.module->lifted_vars.begin() };
        auto last{ b.module->lifted_vars.begin() };
        std::advance(last, b.module->lifted_vars.size() - 1);
        util::format_to(b.footer_buffer,
                        "GC_add_roots(&{}::{}, (&{}::{} + 1));\n",
                        native_ns,
                        first.second.name,
                        native_ns,
                        last->second.name);
      }

      for(auto const &v : b.module->lifted_vars)
      {
        /* Since global ctors don't run when loading object files, we
         * need to manually initialize these. We use placement new to
         * properly run ctors, just like what would happen normally. */
        if(v.second.owned)
        {
          util::format_to(b.footer_buffer,
                          "new (&{}::{}) auto(_jank_var_owned(\"{}\"));\n",
                          native_ns,
                          v.second.name,
                          v.first);
        }
        else
        {
          util::format_to(b.footer_buffer,
                          "new (&{}::{}) auto(_jank_var(\"{}\"));\n",
                          native_ns,
                          v.second.name,
                          v.first);
        }
      }

      if(!b.module->lifted_constants.empty())
      {
        auto const &first{ *b.module->lifted_constants.begin() };
        auto last{ b.module->lifted_constants.begin() };
        std::advance(last, b.module->lifted_constants.size() - 1);
        util::format_to(b.footer_buffer,
                        "GC_add_roots(&{}::{}, (&{}::{} + 1));\n",
                        native_ns,
                        first.second,
                        native_ns,
                        last->second,
                        native_ns,
                        last->second);
      }

      for(auto const &v : b.module->lifted_constants)
      {
        util::format_to(b.footer_buffer, "new (&{}::{}) auto(", native_ns, v.second);
        detail::gen_constant(v.first, b.footer_buffer);
        util::format_to(b.footer_buffer, ");\n");
      }

      auto const fn_tmp{ b.expression_str() };
      util::format_to(b.footer_buffer, "{}->call();\n", fn_tmp);

      /* Load fn. */
      util::format_to(b.footer_buffer, "}\n");

      /* Namespace. */
      util::format_to(b.footer_buffer, "}\n");
    }

    generated_cpp ret{ b.declaration_str(), b.expression_str() };
    //util::println("\n\n{}", util::format_cpp_source(ret.declaration).expect_ok());
    return ret;
  }
}
