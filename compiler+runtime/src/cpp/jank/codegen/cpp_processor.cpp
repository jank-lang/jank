#include <CppInterOp/Compatibility.h>
#include <CppInterOp/CppInterOp.h>

#include <jank/analyze/visit.hpp>
#include <jank/analyze/cpp_util.hpp>
#include <jank/ir/processor.hpp>
#include <jank/ir/visit.hpp>
#include <jank/codegen/cpp_processor.hpp>
#include <jank/codegen/llvm_processor.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/munge.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/core/meta.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/sequence_range.hpp>
#include <jank/util/escape.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/util/clang_format.hpp>

namespace jank::codegen
{
  using namespace analyze::cpp_util;

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
      declaration.reserve(cpp_raw_buffer.size() + deps_buffer.size() + header_buffer.size()
                          + body_buffer.size() + footer_buffer.size());
      declaration += jtl::immutable_string_view{ cpp_raw_buffer.data(), cpp_raw_buffer.size() };
      declaration += jtl::immutable_string_view{ deps_buffer.data(), deps_buffer.size() };
      declaration += jtl::immutable_string_view{ header_buffer.data(), header_buffer.size() };
      declaration += jtl::immutable_string_view{ body_buffer.data(), body_buffer.size() };
      declaration += jtl::immutable_string_view{ footer_buffer.data(), footer_buffer.size() };

      return declaration;
    }

    jtl::immutable_string expression_str() const
    {
      return "";
    }

    jtl::ref<ir::module> module;
    jtl::ref<ir::function> function;

    jtl::string_builder cpp_raw_buffer{};
    jtl::string_builder deps_buffer{};
    jtl::string_builder header_buffer{};
    jtl::string_builder body_buffer{};
    jtl::string_builder footer_buffer{};
    jtl::string_builder expression_buffer{};

    usize block_index{}, instruction_index{};
    native_vector<std::pair<jtl::immutable_string, jtl::immutable_string>> deferred_bindings;
  };

  using identifier = jtl::immutable_string;

  void gen(ir::function const &fn, builder &b);

  namespace detail
  {
    static bool should_gen_meta(object_ref const meta)
    {
      return meta.is_some() && !runtime::is_empty(meta);
    }

    static void gen_constant(runtime::object_ref const o, jtl::string_builder &buffer)
    {
      runtime::visit_object(
        [&](auto const typed_o) {
          using T = typename decltype(typed_o)::value_type;

          if constexpr(std::same_as<T, runtime::obj::nil>)
          {
            util::format_to(buffer, "jank::runtime::jank_nil()");
          }
          else if constexpr(std::same_as<T, runtime::obj::boolean>)
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
          else if constexpr(std::same_as<T, runtime::obj::integer>)
          {
            util::format_to(buffer,
                            "jank::runtime::make_box(static_cast<jank::"
                            "i64>({}))",
                            typed_o->data);
          }
          else if constexpr(std::same_as<T, runtime::obj::real>)
          {
            util::format_to(buffer,
                            "jank::runtime::make_box(static_cast<jank::"
                            "f64>(");

            if(std::isinf(typed_o->data))
            {
              util::format_to(buffer, "INFINITY");
            }
            else if(std::isnan(typed_o->data))
            {
              util::format_to(buffer, "NAN");
            }
            else
            {
              util::format_to(buffer, "{}", typed_o->data);
            }

            util::format_to(buffer, "))");
          }
          else if constexpr(std::same_as<T, runtime::obj::big_integer>)
          {
            util::format_to(buffer,
                            "jank::runtime::make_box<jank::runtime::obj::big_integer>(\"{}\")",
                            typed_o->to_string());
          }
          else if constexpr(std::same_as<T, runtime::obj::big_decimal>)
          {
            util::format_to(buffer,
                            "jank::runtime::make_box<jank::runtime::obj::big_decimal>(\"{}\")",
                            typed_o->to_string());
          }
          else if constexpr(std::same_as<T, runtime::obj::ratio>)
          {
            util::format_to(buffer,
                            "jank::runtime::obj::ratio::create({}, {})",
                            typed_o->data.numerator,
                            typed_o->data.denominator);
          }
          else if constexpr(std::same_as<T, runtime::obj::symbol>)
          {
            if(typed_o->meta.is_some())
            {
              util::format_to(buffer, "jank::runtime::make_box<jank::runtime::obj::symbol>( ");
              gen_constant(typed_o->meta, buffer);
              util::format_to(buffer, R"(, "{}", "{}"))", typed_o->ns, typed_o->name);
            }
            else
            {
              util::format_to(buffer,
                              R"(jank::runtime::make_box<jank::runtime::obj::symbol>("{}", "{}"))",
                              typed_o->ns,
                              typed_o->name);
            }
          }
          else if constexpr(std::same_as<T, runtime::obj::character>)
          {
            util::format_to(buffer,
                            R"(jank::runtime::make_box<jank::runtime::obj::character>("{}"))",
                            util::escape(typed_o->to_string()));
          }
          else if constexpr(std::same_as<T, runtime::obj::keyword>)
          {
            util::format_to(
              buffer,
              R"(jank::runtime::__rt_ctx->intern_keyword("{}", "{}", true).expect_ok())",
              typed_o->sym->ns,
              typed_o->sym->name);
          }
          else if constexpr(std::same_as<T, runtime::obj::re_pattern>)
          {
            util::format_to(buffer,
                            R"(jank::runtime::make_box<jank::runtime::obj::re_pattern>({}))",
                            /* We remove the # prefix here. */
                            typed_o->to_code_string().substr(1));
          }
          else if constexpr(std::same_as<T, runtime::obj::uuid>)
          {
            util::format_to(buffer,
                            R"(jank::runtime::make_box<jank::runtime::obj::uuid>("{}"))",
                            typed_o->to_string());
          }
          else if constexpr(std::same_as<T, runtime::obj::persistent_string>)
          {
            if(typed_o->data.empty())
            {
              util::format_to(buffer, "jank::runtime::obj::persistent_string::empty()");
            }
            else
            {
              util::format_to(buffer,
                              "jank::runtime::make_box<jank::runtime::obj::persistent_string>({})",
                              typed_o->to_code_string());
            }
          }
          else if constexpr(std::same_as<T, runtime::obj::persistent_vector>)
          {
            if(typed_o->data.empty())
            {
              util::format_to(buffer, "jank::runtime::obj::persistent_vector::empty()");
              if(should_gen_meta(typed_o->meta))
              {
                util::format_to(buffer, "->with_meta(");
                gen_constant(typed_o->meta, buffer);
                util::format_to(buffer, ")");
              }
            }
            else
            {
              util::format_to(buffer,
                              "jank::runtime::make_box<jank::runtime::obj::persistent_vector>(");
              if(should_gen_meta(typed_o->meta))
              {
                gen_constant(typed_o->meta, buffer);
                util::format_to(buffer, ",");
              }
              util::format_to(buffer, "std::in_place ");
              for(auto const &form : typed_o->data)
              {
                util::format_to(buffer, ", ");
                gen_constant(form, buffer);
              }
              util::format_to(buffer, ")");
            }
          }
          else if constexpr(std::same_as<T, runtime::obj::persistent_list>)
          {
            if(typed_o->data.empty())
            {
              util::format_to(buffer, "jank::runtime::obj::persistent_list::empty()");
              if(should_gen_meta(typed_o->meta))
              {
                util::format_to(buffer, "->with_meta(");
                gen_constant(typed_o->meta, buffer);
                util::format_to(buffer, ")");
              }
            }
            else
            {
              util::format_to(buffer,
                              "jank::runtime::make_box<jank::runtime::obj::persistent_list>(");
              if(should_gen_meta(typed_o->meta))
              {
                gen_constant(typed_o->meta, buffer);
                util::format_to(buffer, ",");
              }
              util::format_to(buffer, "std::in_place ");
              for(auto const &form : typed_o->data)
              {
                util::format_to(buffer, ", ");
                gen_constant(form, buffer);
              }
              util::format_to(buffer, ")");
            }
          }
          else if constexpr(std::same_as<T, runtime::obj::persistent_hash_set>)
          {
            if(typed_o->data.empty())
            {
              util::format_to(buffer, "jank::runtime::obj::persistent_hash_set::empty()");
              if(should_gen_meta(typed_o->meta))
              {
                util::format_to(buffer, "->with_meta(");
                gen_constant(typed_o->meta, buffer);
                util::format_to(buffer, ")");
              }
            }
            else
            {
              util::format_to(buffer,
                              "jank::runtime::make_box<jank::runtime::obj::persistent_hash_set>(");
              if(should_gen_meta(typed_o->meta))
              {
                gen_constant(typed_o->meta, buffer);
                util::format_to(buffer, ",");
              }
              util::format_to(buffer, "std::in_place ");
              for(auto const &form : typed_o->data)
              {
                util::format_to(buffer, ", ");
                gen_constant(form, buffer);
              }
              util::format_to(buffer, ")");
            }
          }
          else if constexpr(std::same_as<T, runtime::obj::persistent_array_map>)
          {
            if(typed_o->data.empty())
            {
              util::format_to(buffer, "jank::runtime::obj::persistent_array_map::empty()");
              if(should_gen_meta(typed_o->meta))
              {
                util::format_to(buffer, "->with_meta(");
                gen_constant(typed_o->meta, buffer);
                util::format_to(buffer, ")");
              }
            }
            else
            {
              bool need_comma{};
              if(should_gen_meta(typed_o->meta))
              {
                util::format_to(
                  buffer,
                  "jank::runtime::obj::persistent_array_map::create_unique_with_meta(");
                gen_constant(typed_o->meta, buffer);
                need_comma = true;
              }
              else
              {
                util::format_to(buffer, "jank::runtime::obj::persistent_array_map::create_unique(");
              }
              for(auto const &form : typed_o->data)
              {
                if(need_comma)
                {
                  util::format_to(buffer, ", ");
                }
                need_comma = true;
                gen_constant(form.first, buffer);
                util::format_to(buffer, ", ");
                gen_constant(form.second, buffer);
              }
              util::format_to(buffer, ")");
            }
          }
          else if constexpr(std::same_as<T, runtime::obj::persistent_hash_map>)
          {
            if(typed_o->data.empty())
            {
              util::format_to(buffer, "jank::runtime::obj::persistent_hash_map::empty()");
              if(should_gen_meta(typed_o->meta))
              {
                util::format_to(buffer, "->with_meta(");
                gen_constant(typed_o->meta, buffer);
                util::format_to(buffer, ")");
              }
            }
            else
            {
              auto const has_meta{ should_gen_meta(typed_o->meta) };
              if(has_meta)
              {
                util::format_to(buffer, "jank::runtime::with_meta(");
              }
              util::format_to(buffer,
                              "jank::runtime::__rt_ctx->forcefully_read_string(\"{}\")",
                              util::escape(typed_o->to_code_string()));
              if(has_meta)
              {
                util::format_to(buffer, ",");
                gen_constant(typed_o->meta, buffer);
              }
            }
          }
          /* Cons, etc. */
          else if constexpr(runtime::behavior::seqable<T>)
          {
            util::format_to(
              buffer,
              "jank::runtime::make_box<jank::runtime::obj::persistent_list>(std::in_place");
            for(auto const it : runtime::make_sequence_range(typed_o))
            {
              util::format_to(buffer, ", ");
              gen_constant(it, buffer);
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
      if(current_inst->kind == ir::instruction_kind::jump && jump_block.is_some()
         && static_box_cast<ir::inst::jump>(current_inst)->block == jump_block.unwrap())
      {
        break;
      }
      gen(current_inst, b);
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
    util::format_to(
      b.body_buffer,
      R"(auto const {}(jank::runtime::__rt_ctx->intern_owned_var("{}").expect_ok());)",
      inst->name,
      inst->qualified_var);

    /* Forward declarations just intern the var and evaluate to it. */
    if(inst->value.is_none())
    {
      util::format_to(b.body_buffer,
                      "{}->with_meta({})->set_dynamic({});",
                      inst->name,
                      inst->meta,
                      inst->is_dynamic);
      return inst->name;
    }

    util::format_to(b.body_buffer,
                    "{}->bind_root({})->with_meta({})->set_dynamic({});",
                    inst->name,
                    inst->value.unwrap(),
                    inst->meta,
                    inst->is_dynamic);
    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::var_deref_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(
      b.body_buffer,
      "auto const {}(jank::runtime::__rt_ctx->intern_var(\"{}\").expect_ok()->deref());",
      inst->name,
      inst->qualified_var);
    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::var_ref_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(b.body_buffer,
                    "auto const {}(jank::runtime::__rt_ctx->intern_var(\"{}\").expect_ok());",
                    inst->name,
                    inst->qualified_var);
    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::dynamic_call_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(b.body_buffer,
                    "auto const {}(jank::runtime::dynamic_call({}",
                    inst->name,
                    inst->fn);
    for(auto const &arg : inst->args)
    {
      util::format_to(b.body_buffer, ", {}", arg);
    }

    util::format_to(b.body_buffer, "));");
    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::literal_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(b.body_buffer, "auto const {}(", inst->name);
    detail::gen_constant(inst->value, b.body_buffer);
    util::format_to(b.body_buffer, ");");
    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::persistent_list_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(b.body_buffer,
                    "auto const {}(jank::runtime::make_box<jank::runtime::obj::persistent_list>(",
                    inst->name);
    if(inst->meta.is_some())
    {
      util::format_to(b.body_buffer, "{}, ", inst->meta.unwrap());
    }
    util::format_to(b.body_buffer, "std::in_place ");
    for(auto const &val : inst->values)
    {
      util::format_to(b.body_buffer, ", ");
      util::format_to(b.body_buffer, "{}", val);
    }
    util::format_to(b.body_buffer, "));");

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::persistent_vector_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(b.body_buffer,
                    "auto const {}(jank::runtime::make_box<jank::runtime::obj::persistent_vector>(",
                    inst->name);
    if(inst->meta.is_some())
    {
      util::format_to(b.body_buffer, "{}, ", inst->meta.unwrap());
    }
    util::format_to(b.body_buffer, "std::in_place ");
    for(auto const &val : inst->values)
    {
      util::format_to(b.body_buffer, ", ");
      util::format_to(b.body_buffer, "{}", val);
    }
    util::format_to(b.body_buffer, "));");

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::persistent_array_map_ref const &inst, builder &b)
  {
    b.next_instruction();
    if(inst->meta.is_some())
    {
      util::format_to(
        b.body_buffer,
        "auto const {}(jank::runtime::obj::persistent_array_map::create_unique_with_meta({},",
        inst->name,
        inst->meta.unwrap());
    }
    else
    {
      util::format_to(b.body_buffer,
                      "auto const {}(jank::runtime::obj::persistent_array_map::create_unique(",
                      inst->name);
    }

    bool need_comma{};
    for(auto const &val : inst->values)
    {
      if(need_comma)
      {
        util::format_to(b.body_buffer, ", ");
      }
      need_comma = true;
      util::format_to(b.body_buffer, "{}, {}", val.first, val.second);
    }
    util::format_to(b.body_buffer, "));");

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::persistent_hash_map_ref const &inst, builder &b)
  {
    b.next_instruction();
    if(inst->meta.is_some())
    {
      util::format_to(
        b.body_buffer,
        "auto const {}(jank::runtime::obj::persistent_hash_map::create_unique_with_meta({},",
        inst->name,
        inst->meta.unwrap());
    }
    else
    {
      util::format_to(b.body_buffer,
                      "auto const {}(jank::runtime::obj::persistent_hash_map::create_unique(",
                      inst->name);
    }

    bool need_comma{};
    for(auto const &val : inst->values)
    {
      if(need_comma)
      {
        util::format_to(b.body_buffer, ", ");
      }
      need_comma = true;
      util::format_to(b.body_buffer, "{}, {}", val.first, val.second);
    }
    util::format_to(b.body_buffer, "));");

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::persistent_hash_set_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(
      b.body_buffer,
      "auto const {}(jank::runtime::make_box<jank::runtime::obj::persistent_hash_set>(",
      inst->name);
    if(inst->meta.is_some())
    {
      util::format_to(b.body_buffer, "{}, ", inst->meta.unwrap());
    }
    util::format_to(b.body_buffer, "std::in_place ");
    for(auto const &val : inst->values)
    {
      util::format_to(b.body_buffer, ", ");
      util::format_to(b.body_buffer, "{}", val);
    }
    util::format_to(b.body_buffer, "));");

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::function_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(b.body_buffer, "auto const {}(", inst->name);
    util::format_to(b.body_buffer,
                    "jank::runtime::make_box<jank::runtime::obj::jit_function>({})",
                    inst->arity_flags);
    util::format_to(b.body_buffer, ");");

    for(auto const &arity : inst->arities)
    {
      util::format_to(b.body_buffer, "{}->arity_{} = &{};", inst->name, arity.first, arity.second);
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
      util::format_to(b.deps_buffer, "jank::runtime::object_ref {};", capture.first);

      if(need_comma)
      {
        util::format_to(b.body_buffer, ", ");
      }
      if(capture.second == ":defer")
      {
        b.defer(inst->context, capture.first);
        b.body_buffer("jank::runtime::jank_nil()");
      }
      else
      {
        b.body_buffer(capture.second);
      }
    }

    util::format_to(b.deps_buffer, "};");
    util::format_to(b.body_buffer, "));");


    util::format_to(b.body_buffer, "auto const {}(", inst->name);
    util::format_to(b.body_buffer,
                    "jank::runtime::make_box<jank::runtime::obj::jit_closure>({}, {}.data)",
                    inst->arity_flags,
                    inst->context);
    util::format_to(b.body_buffer, ");");

    for(auto const &arity : inst->arities)
    {
      util::format_to(b.body_buffer, "{}->arity_{} = &{};", inst->name, arity.first, arity.second);
      builder nested{ b.module, arity.second };
      gen(*nested.function, nested);
      util::format_to(b.deps_buffer, "{}", nested.declaration_str());
    }

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::parameter_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(b.body_buffer, "auto &&{}({});", inst->name, inst->value);
    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::capture_ref const &inst, builder &b)
  {
    b.next_instruction();
    auto const &closure_ctx{ munge(b.function->arity->fn_ctx->unique_name + "_ctx") };
    util::format_to(b.body_buffer, "auto &&{}({}->{});", inst->name, closure_ctx, inst->value);
    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::recursion_reference_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(b.body_buffer,
                    "auto &&{}({});",
                    inst->name,
                    munge(b.function->arity->fn_ctx->fn->name));
    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::named_recursion_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(b.body_buffer,
                    "auto const {}(jank::runtime::dynamic_call({}",
                    inst->name,
                    inst->fn);
    for(auto const &arg : inst->args)
    {
      util::format_to(b.body_buffer, ", {}", arg);
    }

    util::format_to(b.body_buffer, "));");
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
                      "{}->{} = {};",
                      deferred.first,
                      deferred.second,
                      bindings[deferred.second]);
    }

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::jump_ref const &inst, builder &b)
  {
    b.enter_block(inst->block);
    return none;
  }

  jtl::option<identifier> gen(ir::inst::truthy_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(b.body_buffer,
                    "auto const {}(jank::runtime::truthy({}));",
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
    util::format_to(b.body_buffer, "{} = {};", inst->shadow, inst->value);
    return none;
  }

  jtl::option<identifier> gen(ir::inst::branch_ref const &inst, builder &b)
  {
    if(inst->shadow.is_some())
    {
      util::format_to(b.body_buffer,
                      "{} {}{ };",
                      get_qualified_type_name(inst->shadow.unwrap().type),
                      inst->shadow.unwrap().name);
    }

    util::format_to(b.body_buffer, "if({}){ ", inst->condition);
    b.enter_block(inst->then_block);
    gen_until_jump(inst->merge_block, b);

    util::format_to(b.body_buffer, "} else {");
    b.enter_block(inst->else_block);
    gen_until_jump(inst->merge_block, b);
    util::format_to(b.body_buffer, "}");

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
                    "throw static_cast<jank::runtime::object_ref>({});",
                    inst->value);
    return none;
  }

  jtl::option<identifier> gen(ir::inst::try_ref const &inst, builder &b)
  {
    b.next_instruction();
    auto const has_catch{ !inst->catches.empty() };
    if(inst->shadow.is_some())
    {
      util::format_to(b.body_buffer, "jank::runtime::object_ref {};", inst->shadow.unwrap());
    }

    if(has_catch)
    {
      util::format_to(b.body_buffer, "try {");

      if(inst->finally_block.is_some())
      {
        auto const instruction_index{ b.instruction_index };
        auto const block_index{ b.block_index };
        b.enter_block(inst->finally_block.unwrap());

        gen_until_jump(inst->merge_block, b);

        b.instruction_index = instruction_index;
        b.block_index = block_index;
      }

      auto const &jump_block{ inst->finally_block.is_some() ? inst->finally_block
                                                            : inst->merge_block };

      gen_until_jump(jump_block, b);

      util::format_to(b.body_buffer, "}");
      for(auto const &catch_details : inst->catches)
      {
        b.enter_block(catch_details.second);
        gen_until_jump(jump_block, b);
      }

      if(inst->merge_block.is_some())
      {
        b.enter_block(inst->merge_block.unwrap());
      }
    }
    else
    {
      if(inst->finally_block.is_some())
      {
        auto const instruction_index{ b.instruction_index };
        auto const block_index{ b.block_index };
        b.enter_block(inst->finally_block.unwrap());

        util::format_to(b.body_buffer, "{");
        gen_until_jump(inst->merge_block, b);
        util::format_to(b.body_buffer, "}");

        b.instruction_index = instruction_index;
        b.block_index = block_index;
      }

      auto const &jump_block{ inst->finally_block.is_some() ? inst->finally_block
                                                            : inst->merge_block };

      gen_until_jump(jump_block, b);

      if(inst->finally_block.is_some())
      {
        b.enter_block(inst->merge_block.unwrap());
      }
    }

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
    util::format_to(b.body_buffer, "}");
    return none;
  }

  jtl::option<identifier> gen(ir::inst::finally_ref const &inst, builder &b)
  {
    b.next_instruction();

    auto const fn_name{ inst->name + "_fn" };
    util::format_to(b.body_buffer, "auto const {}{ [&](){ ", fn_name);
    gen_until_jump(inst->merge_block, b);
    util::format_to(b.body_buffer, "} };");
    util::format_to(b.body_buffer, "jank::util::scope_exit {}{ {}, true };", inst->name, fn_name);
    return none;
  }

  jtl::option<identifier> gen(ir::inst::case_ref const &inst, builder &b)
  {
    b.next_instruction();

    if(inst->shadow.is_some())
    {
      util::format_to(b.body_buffer, "jank::runtime::object_ref {};", inst->shadow.unwrap());
    }

    util::format_to(b.body_buffer,
                    "switch(jank_shift_mask_case_integer({}.get(), {}, {})) {",
                    inst->value,
                    inst->shift,
                    inst->mask);

    for(auto const &case_block : inst->case_blocks)
    {
      util::format_to(b.body_buffer, "case {}: {", case_block.first);
      b.enter_block(case_block.second);
      gen_until_jump(inst->merge_block, b);
      util::format_to(b.body_buffer, "break; }");
    }

    util::format_to(b.body_buffer, "default: {");
    b.enter_block(inst->default_block);
    gen_until_jump(inst->merge_block, b);
    util::format_to(b.body_buffer, "break; }");

    util::format_to(b.body_buffer, "}");

    if(inst->merge_block.is_some())
    {
      b.enter_block(inst->merge_block.unwrap());
    }

    return none;
  }

  jtl::option<identifier> gen(ir::inst::ret_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(b.body_buffer, "return {};", inst->value);

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
      return "nullptr";
    }
    if(inst->expr->val_kind == analyze::expr::cpp_value::value_kind::bool_true
       || inst->expr->val_kind == analyze::expr::cpp_value::value_kind::bool_false)
    {
      auto const val{ inst->expr->val_kind == analyze::expr::cpp_value::value_kind::bool_true };
      return util::format("{}", val);
    }

    util::format_to(b.body_buffer,
                    "auto &&{}({}{});",
                    inst->name,
                    (Cpp::IsPointerToMemberType(inst->expr->type) ? "&" : ""),
                    Cpp::GetQualifiedCompleteNameWithTemplateArgs(inst->expr->scope));

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
      util::format_to(b.body_buffer, "auto const {}(jank::runtime::jank_nil());", inst->name);
      return inst->name;
    }

    /* We can rely on the C++ type system to handle conversion from typed objects
     * to untype objects. */
    if(is_untyped_object(inst->expr->type) && is_any_object(inst->expr->conversion_type))
    {
      return inst->value;
    }

    util::format_to(b.body_buffer,
                    "auto const {}(jank::runtime::convert<{}>::{}({}));",
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
    b.next_instruction();
    ir::inst::cpp_into_object from{ inst->name, inst->value, inst->expr };
    return gen(ir::inst::cpp_into_object_ref{ &from }, b);
  }

  jtl::option<identifier> gen(ir::inst::cpp_unsafe_cast_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(b.body_buffer,
                    "auto const {}(({})({}));",
                    inst->name,
                    get_qualified_type_name(inst->expr->type),
                    inst->value);

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::cpp_call_ref const &inst, builder &b)
  {
    b.next_instruction();
    //if((target == compilation_target::module || target == compilation_target::module_function)
    //   && !expr->function_code.empty())
    //{
    //  util::format_to(cpp_raw_buffer, "\n{}\n", expr->function_code);
    //}

    auto const source_type{ expression_type(inst->expr->source_expr) };

    if(inst->expr->source_expr->kind == analyze::expression_kind::cpp_value)
    {
      auto const source{ static_cast<analyze::expr::cpp_value *>(inst->expr->source_expr.data) };

      auto const is_void{ Cpp::IsVoid(Cpp::GetFunctionReturnType(source->scope)) };
      if(is_void)
      {
        util::format_to(b.body_buffer, "jank::runtime::object_ref {};", inst->name);
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
          util::format_to(b.body_buffer, ".get()");
        }
        need_comma = true;
      }

      util::format_to(b.body_buffer, ")");

      if(!is_void)
      {
        util::format_to(b.body_buffer, ");");
      }
      else
      {
        util::format_to(b.body_buffer, ";");
      }

      return inst->name;
    }
    else if(Cpp::IsPointerToMemberVariableType(source_type))
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
        util::format_to(b.body_buffer, ");");
      }
      else
      {
        util::format_to(b.body_buffer, ";");
      }

      return inst->name;
    }
    else if(Cpp::IsPointerToMemberFunctionType(source_type))
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
        util::format_to(b.body_buffer, ");");
      }
      else
      {
        util::format_to(b.body_buffer, ";");
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
        util::format_to(b.body_buffer, ");");
      }
      else
      {
        util::format_to(b.body_buffer, ";");
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
        util::format_to(b.body_buffer, "){ };");
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
          "{} ({}{})[{}]{ };",
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
                        "{} {}{ };",
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
                        "jank::runtime::convert<{}>::{}({}.get())",
                        trait_type,
                        conversion_direction,
                        inst->args[0]);
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

    util::format_to(b.body_buffer, "{};", (inst->expr->is_aggregate ? "}" : ")"));

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::cpp_member_call_ref const &inst, builder &b)
  {
    b.next_instruction();
    auto const fn_name{ Cpp::GetName(inst->expr->fn) };
    auto const is_void{ Cpp::IsVoid(Cpp::GetFunctionReturnType(inst->expr->fn)) };

    if(is_void)
    {
      util::format_to(b.body_buffer, "jank::runtime::object_ref {}{ };", inst->name);
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
      util::format_to(b.body_buffer, ");");
    }
    else
    {
      util::format_to(b.body_buffer, "));");
    }

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::cpp_member_access_ref const &inst, builder &b)
  {
    b.next_instruction();
    util::format_to(b.body_buffer,
                    "auto &&{}({}{}{});",
                    inst->name,
                    inst->value,
                    (Cpp::IsPointerType(expression_type(inst->expr->obj_expr)) ? "->" : "."),
                    inst->expr->name);

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::cpp_builtin_operator_call_ref const &inst, builder &b)
  {
    b.next_instruction();
    auto const op_name{ operator_name(static_cast<Cpp::Operator>(inst->expr->op)).unwrap() };

    if(inst->args.size() == 1)
    {
      util::format_to(b.body_buffer, "auto &&{}( {}{} );", inst->name, op_name, inst->args[0]);
    }
    else if(op_name == "aget")
    {
      util::format_to(b.body_buffer,
                      "auto &&{}( {}[{}] );",
                      inst->name,
                      inst->args[0],
                      inst->args[1]);
    }
    else
    {
      util::format_to(b.body_buffer,
                      "auto &&{}( {} {} {} );",
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

    auto const meta{ runtime::source_to_meta(inst->expr->source) };
    util::format_to(
      b.body_buffer,
      "jank::runtime::reset_meta({}, jank::runtime::__rt_ctx->forcefully_read_string(\"{}\"));",
      inst->name,
      util::escape(runtime::to_code_string(meta)));

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::cpp_unbox_ref const &inst, builder &b)
  {
    b.next_instruction();
    auto const type_name{ get_qualified_type_name(Cpp::GetCanonicalType(inst->expr->type)) };
    util::format_to(b.body_buffer,
                    "auto {}{ "
                    "static_cast<{}>(jank_unbox_with_source(\"{}\", {}.data, {}.data)) };",
                    inst->name,
                    type_name,
                    type_name,
                    inst->value,
                    meta);

    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::cpp_new_ref const &inst, builder &b)
  {
    b.next_instruction();
    auto finalizer_name{ runtime::munge(__rt_ctx->unique_string("finalizer")) };
    auto const type_name{ get_qualified_type_name(inst->expr->type) };
    auto const needs_finalizer{ !Cpp::IsTriviallyDestructible(inst->expr->type) };

    if(needs_finalizer)
    {
      util::format_to(b.body_buffer,
                      "using T = {};\n"
                      "static auto const {}("
                      "[](void * const obj, void *){"
                      "reinterpret_cast<T*>(obj)->~T();"
                      "});",
                      type_name,
                      finalizer_name);
    }

    util::format_to(b.body_buffer,
                    "auto {}{ "
                    "new (UseGC{}) {}{ {} }"
                    " };",
                    inst->name,
                    (needs_finalizer ? ", " + finalizer_name : ""),
                    type_name,
                    inst->value);

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
                      "using T = {};\n"
                      "{}->~T();",
                      type_name,
                      inst->value);
    }

    util::format_to(b.body_buffer, "GC_free({});", inst->value);

    return "jank::runtime::jank_nil()";
  }

  jtl::option<identifier> gen(ir::instruction_ref const &inst, builder &b)
  {
    jtl::string_builder sb;
    inst->print(sb, 0);
    util::println("gen {}", sb.release());
    jtl::option<identifier> name;
    ir::visit_inst([&](auto const typed_inst) { name = gen(typed_inst, b); }, inst);
    return name;
  }

  void gen(ir::function const &fn, builder &b)
  {
    auto const &all_captures{ fn.arity->frame->captures };
    auto const &munged_fn_name{ munge(fn.arity->fn_ctx->name) };
    auto const &munged_linkage_name{ munge(fn.arity->fn_ctx->unique_name) };
    auto const &closure_ctx{ munge(fn.arity->fn_ctx->unique_name + "_ctx") };

    bool param_shadows_fn{};
    for(auto const &param : fn.arity->params)
    {
      param_shadows_fn |= param->name == fn.arity->fn_ctx->name;
    }

    util::format_to(
      b.body_buffer,
      "extern \"C\" jank::runtime::object_ref {}_{}(jank::runtime::object_ref const {}",
      munged_linkage_name,
      fn.arity->params.size(),
      param_shadows_fn ? "" : munged_fn_name);

    for(auto const &param : fn.arity->params)
    {
      util::format_to(b.body_buffer, ", jank::runtime::object_ref {}", munge(param->name));
    }

    util::format_to(b.body_buffer, ") {");

    //util::format_to(body_buffer, "jank::profile::timer __timer{ \"{}\" };", root_fn->name);

    if(!all_captures.empty())
    {
      util::format_to(b.body_buffer,
                      "auto const * const {}{ "
                      "static_cast<struct {}*>(static_cast<jank::runtime::obj::jit_"
                      "closure*>({}.data)->context) };",
                      closure_ctx,
                      closure_ctx,
                      munged_fn_name);
    }

    if(fn.arity->fn_ctx->is_recur_recursive)
    {
      util::format_to(b.body_buffer, "{");

      for(auto const &param : fn.arity->params)
      {
        auto const name{ munge(param->name) };
        util::format_to(b.body_buffer, "auto {}({});", name, name);
      }

      util::format_to(b.body_buffer,
                      R"(
            while(true)
            {
          )");
    }

    b.block_index = 0;
    b.instruction_index = 0;
    while(b.instruction_index < b.function->blocks[b.block_index].instructions.size())
    {
      gen(b.function->blocks[b.block_index].instructions[b.instruction_index], b);
    }

    if(fn.arity->body->values.empty())
    {
      util::format_to(b.body_buffer, "return { };");
    }

    if(fn.arity->fn_ctx->is_recur_recursive)
    {
      util::format_to(b.body_buffer, "} }");
    }

    util::format_to(b.body_buffer, "}");
  }

  generated_cpp gen_cpp(ir::module const &mod)
  {
    builder b{ &mod, mod.entry_points[0] };

    auto const arity_flags{ find_function(&mod, mod.entry_points[0])->arity->fn_ctx->fn->arities };
    for(auto const &fn_name : mod.entry_points)
    {
      auto const fn{ find_function(&mod, fn_name) };
      b.function = fn;
      gen(*fn, b);
    }

    generated_cpp ret{ b.declaration_str(), b.expression_str() };
    util::println("\n\n{}", util::format_cpp_source(ret.declaration).expect_ok());
    return ret;
  }
}
