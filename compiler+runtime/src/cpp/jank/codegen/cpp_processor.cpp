#include <jank/analyze/visit.hpp>
#include <jank/ir/processor.hpp>
#include <jank/ir/visit.hpp>
#include <jank/codegen/cpp_processor.hpp>
#include <jank/codegen/llvm_processor.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/munge.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/sequence_range.hpp>
#include <jank/util/escape.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/util/clang_format.hpp>

namespace jank::codegen
{
  struct builder
  {
    jtl::ref<ir::module> module;

    jtl::string_builder cpp_raw_buffer{};
    jtl::string_builder deps_buffer{};
    jtl::string_builder header_buffer{};
    jtl::string_builder body_buffer{};
    jtl::string_builder footer_buffer{};
    jtl::string_builder expression_buffer{};
  };

  using identifier = jtl::immutable_string;

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

  jtl::option<identifier> gen(ir::inst::def_ref const &inst, builder &b)
  {
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

  jtl::option<identifier> gen(ir::inst::var_deref_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::var_ref_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::dynamic_call_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::literal_ref const &inst, builder &b)
  {
    util::format_to(b.body_buffer, "auto const {}(", inst->name);
    detail::gen_constant(inst->value, b.body_buffer);
    util::format_to(b.body_buffer, ");");
    return inst->name;
  }

  jtl::option<identifier> gen(ir::inst::persistent_list_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::persistent_vector_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::persistent_array_map_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::persistent_hash_map_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::persistent_hash_set_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::function_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::closure_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::recursion_reference_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::named_recursion_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::letfn_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::jump_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::branch_get_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::branch_set_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::branch_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::throw_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::try_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::catch_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::case_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::ret_ref const &inst, builder &b)
  {
    util::format_to(b.body_buffer, "return {};", inst->value);

    return none;
  }

  jtl::option<identifier> gen(ir::inst::cpp_raw_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::cpp_value_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::cpp_into_object_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::cpp_from_object_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::cpp_unsafe_cast_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::cpp_call_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::cpp_constructor_call_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::cpp_member_call_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::cpp_member_access_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::cpp_builtin_operator_call_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::cpp_box_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::cpp_unbox_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::cpp_new_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::inst::cpp_delete_ref const &, builder &)
  {
    return none;
  }

  jtl::option<identifier> gen(ir::instruction_ref const &inst, builder &b)
  {
    jtl::option<identifier> name;
    ir::visit_inst([&](auto const typed_inst) { name = gen(typed_inst, b); }, inst);
    return name;
  }

  void gen_block(ir::function const &,
                 ir::block const &block,
                 builder &b,
                 native_set<jtl::ref<ir::block>> &seen_blocks)
  {
    if(seen_blocks.contains(&block))
    {
      return;
    }

    for(auto const &inst : block.instructions)
    {
      gen(inst, b);
    }
  }

  void gen(ir::function const &fn, builder &b)
  {
    auto const &all_captures{ fn.arity->frame->captures };
    auto const &fn_name{ fn.arity->fn_ctx->name };
    auto const &munged_fn_name{ munge(fn.arity->fn_ctx->name) };
    auto const &munged_linkage_name{ munge(fn.arity->fn_ctx->unique_name) };
    auto const &closure_ctx{ munge(fn_name + "_ctx") };

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

      for(auto const &capture : all_captures)
      {
        /* We have an analysis bug which causes letfn named recursion references to
         * end up being captures, since they're found as locals. This works around issue
         * for now. */
        if(capture.second.binding.native_name == fn_name)
        {
          continue;
        }

        auto const capture_name{ munge(capture.second.binding.native_name) };
        util::format_to(b.body_buffer,
                        "auto &&{}{ {}->{} };",
                        capture_name,
                        closure_ctx,
                        capture_name);
      }
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

    native_set<jtl::ref<ir::block>> seen_blocks;
    gen_block(fn, fn.blocks[0], b, seen_blocks);

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

  static jtl::immutable_string gen_declaration(builder &b)
  {
    native_transient_string declaration;
    declaration.reserve(b.cpp_raw_buffer.size() + b.deps_buffer.size() + b.header_buffer.size()
                        + b.body_buffer.size() + b.footer_buffer.size());
    declaration += jtl::immutable_string_view{ b.cpp_raw_buffer.data(), b.cpp_raw_buffer.size() };
    declaration += jtl::immutable_string_view{ b.deps_buffer.data(), b.deps_buffer.size() };
    declaration += jtl::immutable_string_view{ b.header_buffer.data(), b.header_buffer.size() };
    declaration += jtl::immutable_string_view{ b.body_buffer.data(), b.body_buffer.size() };
    declaration += jtl::immutable_string_view{ b.footer_buffer.data(), b.footer_buffer.size() };

    util::println("\n\n{}", util::format_cpp_source(declaration).expect_ok());

    return declaration;
  }

  static jtl::immutable_string gen_expresssion(builder &)
  {
    return "";
  }

  generated_cpp gen_cpp(ir::module const &mod)
  {
    builder b{ &mod };

    for(auto const &fn : mod.functions)
    {
      gen(fn, b);
    }

    return { gen_declaration(b), gen_expresssion(b) };
  }
}
