#include <algorithm>

#include <clang/Interpreter/CppInterOp.h>
#include <clang/Sema/Sema.h>
#include <Interpreter/Compatibility.h>

#include <jank/analyze/cpp_util.hpp>
#include <jank/analyze/visit.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/munge.hpp>
#include <jank/util/fmt/print.hpp>
#include <jank/util/scope_exit.hpp>
#include <jank/error/analyze.hpp>
#include <jank/error/codegen.hpp>

namespace jank::analyze::cpp_util
{
  /* Even with a SFINAE trap, Clang can get into a bad state when failing to instantiate
   * templates. In that bad state, whatever the next thing is that we parse fails. So, we
   * hack around this by trying to detect that state and them just giving Clang something
   * very easy to parse and being ok with it failing.
   *
   * After that failure, Clang gets back into a good state. */
  static void reset_sfinae_state()
  {
    static_cast<void>(runtime::__rt_ctx->jit_prc.interpreter->Parse("1"));
  }

  jtl::string_result<void> instantiate_if_needed(jtl::ptr<void> const scope)
  {
    if(!scope)
    {
      return ok();
    }

    /* If we have a template specialization and we want to access one of its members, we
     * need to be sure that it's fully instantiated. If we don't, the member won't
     * be found. */
    if(Cpp::IsTemplateSpecialization(scope) || Cpp::IsTemplatedFunction(scope))
    {
      //util::println("instantiating {}", get_qualified_name(scope));
      /* TODO: Get template arg info and specify all of it? */
      if(Cpp::InstantiateTemplate(scope))
      {
        reset_sfinae_state();
        return err("Unable to instantiate template.");
      }

      if(Cpp::IsTemplatedFunction(scope))
      {
        return instantiate_if_needed(Cpp::GetScopeFromType(Cpp::GetFunctionReturnType(scope)));
      }
    }
    else
    {
      //util::println("not instantiating {}", get_qualified_name(scope));
    }

    return ok();
  }

  jtl::ptr<void> apply_pointers(jtl::ptr<void> type, u8 ptr_count)
  {
    while(ptr_count != 0)
    {
      type = Cpp::GetPointerType(type);
      --ptr_count;
    }
    return type;
  }

  jtl::string_result<jtl::ptr<void>> resolve_literal_type(jtl::immutable_string const &literal);

  jtl::ptr<void> resolve_type(jtl::immutable_string const &sym, u8 const ptr_count)
  {
    /* Clang canonicalizes "char" to "signed char" on some platforms, which breaks exception
     * handling since they are distinct types. We use resolve_literal_type to get the
     * exact type for "char". */
    if(sym == "char")
    {
      if(auto const res = resolve_literal_type("char"); res.is_ok())
      {
        return apply_pointers(res.expect_ok(), ptr_count);
      }
    }

    auto const type{ Cpp::GetType(sym) };
    if(type)
    {
      return apply_pointers(type, ptr_count);
    }
    return type;
  }

  /* Resolves the specified dot-separated symbol into its scope.
   *
   * For example, `std.string.iterator` gives us the scope for iterator in std::string.
   *
   * This doesn't work on built-in types, such as `int`, since they don't have a scope.
   *
   * When resolving the scope for an overloaded function, this is tricksy. We just
   * return the scope of the first overload, whatever that is. Then, when we analyze
   * C++ function calls, we end up looking for all functions within the parent scope
   * of the one we chose.
   */
  jtl::string_result<jtl::ptr<void>> resolve_scope(jtl::immutable_string const &sym)
  {
    jtl::ptr<void> scope{ Cpp::GetGlobalScope() };
    usize new_start{};
    while(true)
    {
      auto const dot{ sym.find('.', new_start) };
      if(dot == jtl::immutable_string::npos)
      {
        if(auto const res = instantiate_if_needed(scope); res.is_err())
        {
          return res.expect_err();
        }
        auto const old_scope{ scope };
        auto const subs{ sym.substr(new_start) };
        /* Finding dots will still leave us with the last part of the symbol to lookup. */
        scope = Cpp::GetNamed(subs, scope);
        if(!scope)
        {
          auto const fns{ Cpp::GetFunctionsUsingName(old_scope, subs) };
          if(fns.empty())
          {
            auto const old_scope_name{ Cpp::GetQualifiedName(old_scope) };
            if(old_scope_name.empty())
            {
              return err(util::format("Unable to find '{}' within the global namespace.", subs));
            }
            else
            {
              return err(
                util::format("Unable to find '{}' within namespace '{}'.", subs, old_scope_name));
            }
          }
          if(auto const res = instantiate_if_needed(fns[0]); res.is_err())
          {
            return res.expect_err();
          }

          return fns[0];
        }
        break;
      }
      auto const subs{ sym.substr(new_start, dot - new_start) };
      new_start = dot + 1;
      auto const old_scope{ scope };
      scope = Cpp::GetUnderlyingScope(Cpp::GetNamed(subs, scope));
      if(!scope)
      {
        return err(
          util::format("Unable to find '{}' within namespace '{}' while trying to resolve '{}'.",
                       subs,
                       Cpp::GetQualifiedName(old_scope),
                       sym));
      }
    }

    if(auto const res = instantiate_if_needed(scope); res.is_err())
    {
      return res.expect_err();
    }

    return ok(scope);
  }

  jtl::string_result<jtl::ptr<void>> resolve_literal_type(jtl::immutable_string const &literal)
  {
    auto &diag{ runtime::__rt_ctx->jit_prc.interpreter->getCompilerInstance()->getDiagnostics() };
    clang::DiagnosticErrorTrap const trap{ diag };

    auto const alias{ runtime::__rt_ctx->unique_namespaced_string() };
    /* We add a new line so that a trailing // comment won't interfere with our code. */
    auto const code{ util::format("using {} = {}\n;", runtime::munge(alias), literal) };
    auto parse_res{ runtime::__rt_ctx->jit_prc.interpreter->Parse(code.c_str()) };
    if(!parse_res || trap.hasErrorOccurred())
    {
      reset_sfinae_state();
      return err("Unable to parse C++ literal.");
    }

    auto const * const translation_unit{ parse_res->TUPart };
    auto const size{ std::distance(translation_unit->decls_begin(),
                                   translation_unit->decls_end()) };
    if(size == 0)
    {
      return err("Invalid C++ literal.");
    }
    if(size != 1)
    {
      return err("Extra expressions found in C++ literal.");
    }
    auto const alias_decl{ llvm::cast<clang::TypeAliasDecl>(*translation_unit->decls_begin()) };
    auto const type{ alias_decl->getUnderlyingType().getAsOpaquePtr() };
    auto const scope{ Cpp::GetScopeFromType(type) };

    if(auto const res = instantiate_if_needed(scope); res.is_err())
    {
      return res.expect_err();
    }

    return type;
  }

  /* Resolving arbitrary literal C++ values is a difficult task. Firstly, we need to handle
   * invalid input gracefully and detect common issues. Secondly, we need to handle all sorts
   * of values, include functions, enums, class types, as well as primitives. Thirdly, we need
   * to maintain the reference/pointer/cv info of the original value.
   *
   * We accomplish this in two steps.
   *
   * The first step happens here, which is to take the arbitrary C++ string and build a
   * function around it. We then compile that function so we can later build a call to it and
   * know its final type. This is sufficient for evaluation.
   *
   * The second steps happens in codegen, where we need to copy the function code for this
   * value literal into the module we're building. We only do this during AOT compilation, since
   * it would otherwise cause an ODR violation. However, AOT builds still need that code, so
   * it has to be there. */
  jtl::string_result<literal_value_result>
  resolve_literal_value(jtl::immutable_string const &literal)
  {
    auto &diag{ runtime::__rt_ctx->jit_prc.interpreter->getCompilerInstance()->getDiagnostics() };
    clang::DiagnosticErrorTrap const trap{ diag };

    auto const alias{ runtime::__rt_ctx->unique_namespaced_string() };
    /* We add a new line so that a trailing // comment won't interfere with our code. */
    auto const code{ util::format(
      "[[gnu::always_inline]] inline decltype(auto) {}(){ return ({}\n); }",
      runtime::munge(alias),
      literal) };
    //util::println("cpp/value code: {}", code);
    auto parse_res{ runtime::__rt_ctx->jit_prc.interpreter->Parse(code.c_str()) };
    if(!parse_res || trap.hasErrorOccurred())
    {
      return err("Unable to parse C++ literal.");
    }

    /* TODO: Can we do a reliable size check for extra expressions? */
    auto const * const translation_unit{ parse_res->TUPart };
    auto const size{ std::distance(translation_unit->decls_begin(),
                                   translation_unit->decls_end()) };
    if(size == 0)
    {
      return err("Invalid C++ literal.");
    }

    auto exec_res{ runtime::__rt_ctx->jit_prc.interpreter->Execute(*parse_res) };
    if(exec_res)
    {
      return err("Unable to load C++ literal.");
    }

    auto const f_decl{ llvm::cast<clang::FunctionDecl>(*translation_unit->decls_begin()) };
    auto const ret_type{ Cpp::GetFunctionReturnType(f_decl) };
    if(auto const ret_scope = Cpp::GetScopeFromType(ret_type))
    {
      if(auto const res = instantiate_if_needed(ret_scope); res.is_err())
      {
        return res.expect_err();
      }
    }

    return literal_value_result{ f_decl, ret_type, code };
  }

  /* When we're looking up operator usage, for example, we need to look in the scopes for
   * all of the arguments, including their parent scopes, all the way up to the gobal scope.
   * The operator could be defined in any of those scopes. This function, given some starting
   * scopes, will fill in the rest.
   *
   * The output may contain duplicates. */
  native_vector<jtl::ptr<void>> find_adl_scopes(native_vector<jtl::ptr<void>> const &starters)
  {
    native_vector<jtl::ptr<void>> ret;
    for(auto const scope : starters)
    {
      if(!scope)
      {
        continue;
      }

      ret.emplace_back(scope);
      for(auto s{ Cpp::GetParentScope(scope) }; s != nullptr; s = Cpp::GetParentScope(s))
      {
        ret.emplace_back(s);
      }
    }
    return ret;
  }

  /* For some scopes, CppInterOp will give an <unnamed> result here. That's not
   * helpful for error reporting, so we turn that into the full type name if
   * needed. */
  jtl::immutable_string get_qualified_name(jtl::ptr<void> const scope)
  {
    auto res{ Cpp::GetQualifiedCompleteName(scope) };
    if(res == "<unnamed>")
    {
      res = Cpp::GetTypeAsString(Cpp::GetTypeFromScope(scope));
    }
    return res;
  }

  /* This is a quick and dirty helper to get the RTTI for a given QualType. We need
   * this for exception catching. */
  void register_rtti(jtl::ptr<void> const type)
  {
    auto &diag{ runtime::__rt_ctx->jit_prc.interpreter->getCompilerInstance()->getDiagnostics() };
    clang::DiagnosticErrorTrap const trap{ diag };
    auto const alias{ runtime::__rt_ctx->unique_namespaced_string() };
    auto const code{ util::format("&typeid({})", Cpp::GetTypeAsString(type)) };
    clang::Value value;
    auto exec_res{ runtime::__rt_ctx->jit_prc.interpreter->ParseAndExecute(code.c_str(), &value) };
    if(exec_res || trap.hasErrorOccurred())
    {
      throw error::internal_codegen_failure(
        util::format("Unable to get RTTI for '{}'.", Cpp::GetTypeAsString(type)));
    }

    auto const lljit{ runtime::__rt_ctx->jit_prc.interpreter->getExecutionEngine() };
    llvm::orc::SymbolMap symbols;
    llvm::orc::MangleAndInterner interner{ lljit->getExecutionSession(), lljit->getDataLayout() };
    auto const &symbol{ Cpp::MangleRTTI(type) };
    symbols[interner(symbol)] = llvm::orc::ExecutorSymbolDef(
      llvm::orc::ExecutorAddr(llvm::pointerToJITTargetAddress(value.getPtr())),
      llvm::JITSymbolFlags());

    auto res{ lljit->getMainJITDylib().define(llvm::orc::absoluteSymbols(symbols)) };
    /* We may have duplicate definitions of RTTI in some circumstances, but we
     * can just ignore those. */
    llvm::consumeError(jtl::move(res));
  }

  jtl::ptr<void> untyped_object_ptr_type()
  {
    static jtl::ptr<void> const ret{ Cpp::GetPointerType(Cpp::GetTypeFromScope(
      Cpp::GetNamed("object", Cpp::GetNamed("runtime", Cpp::GetNamed("jank"))))) };
    return ret;
  }

  jtl::ptr<void> untyped_object_ref_type()
  {
    static jtl::ptr<void> const ret{ Cpp::GetCanonicalType(
      Cpp::GetTypeFromScope(Cpp::GetScopeFromCompleteName("jank::runtime::object_ref"))) };
    return ret;
  }

  bool is_member_function(jtl::ptr<void> const scope)
  {
    return Cpp::IsMethod(scope) && !Cpp::IsConstructor(scope) && !Cpp::IsDestructor(scope);
  }

  bool is_non_static_member_function(jtl::ptr<void> const scope)
  {
    return is_member_function(scope) && !Cpp::IsStaticMethod(scope);
  }

  bool is_nullptr(jtl::ptr<void> const type)
  {
    static jtl::ptr<void> const ret{ Cpp::GetCanonicalType(
      Cpp::GetTypeFromScope(Cpp::GetScopeFromCompleteName("std::nullptr_t"))) };
    return Cpp::GetCanonicalType(type) == ret;
  }

  bool is_implicitly_convertible(jtl::ptr<void> const from, jtl::ptr<void> const to)
  {
    auto const from_no_ref{ Cpp::GetCanonicalType(Cpp::GetNonReferenceType(from)) };
    auto const to_no_ref{ Cpp::GetCanonicalType(Cpp::GetNonReferenceType(to)) };
    if(from_no_ref == to_no_ref || from_no_ref == Cpp::GetTypeWithoutCv(to_no_ref))
    {
      return true;
    }

    return Cpp::IsImplicitlyConvertible(from, to);
  }

  bool is_untyped_object(jtl::ptr<void> const type)
  {
    auto const can_type{ Cpp::GetCanonicalType(
      Cpp::GetTypeWithoutCv(Cpp::GetNonReferenceType(type))) };
    return can_type == untyped_object_ptr_type() || can_type == untyped_object_ref_type();
  }

  static jtl::ptr<void> oref_template()
  {
    static jtl::ptr<void> const ret{ Cpp::GetUnderlyingScope(
      Cpp::GetScopeFromCompleteName("jank::runtime::oref")) };
    return ret;
  }

  /* TODO: Support for typed object raw pointers. */
  bool is_typed_object(jtl::ptr<void> const type)
  {
    auto const can_type{ Cpp::GetCanonicalType(
      Cpp::GetTypeWithoutCv(Cpp::GetNonReferenceType(type))) };
    /* TODO: Need underlying? */
    auto const scope{ Cpp::GetUnderlyingScope(Cpp::GetScopeFromType(can_type)) };
    return !is_untyped_object(can_type) && scope
      && Cpp::IsTemplateSpecializationOf(scope, oref_template());
  }

  bool is_any_object(jtl::ptr<void> type)
  {
    return is_untyped_object(type) || is_typed_object(type);
  }

  /* Clang treats int, long, float, etc as built-in, but not pointers. I consider
   * all of them built-in, but to not confuse the nomenclature, I'm calling them
   * primitives instead. */
  bool is_primitive(jtl::ptr<void> const type)
  {
    return Cpp::IsBuiltin(type) || Cpp::IsPointerType(type) || Cpp::IsArrayType(type)
      || Cpp::IsEnumType(type);
  }

  /* TODO: Just put a type member function in expression_base and read it from there. */
  jtl::ptr<void> expression_type(expression_ref const expr)
  {
    return visit_expr(
      [](auto const typed_expr) -> jtl::ptr<void> {
        using T = typename decltype(typed_expr)::value_type;

        if constexpr(jtl::is_same<T, expr::cpp_new>)
        {
          return Cpp::GetPointerType(typed_expr->type);
        }
        else if constexpr(requires(T *t) {
                            { t->type } -> jtl::is_convertible<jtl::ptr<void>>;
                          })
        {
          return typed_expr->type;
        }
        else if constexpr(jtl::is_same<T, expr::cpp_member_call>)
        {
          return Cpp::GetFunctionReturnType(typed_expr->fn);
        }
        else if constexpr(jtl::is_same<T, expr::local_reference>)
        {
          return typed_expr->binding->type;
        }
        else if constexpr(jtl::is_same<T, expr::let> || jtl::is_same<T, expr::letfn>)
        {
          return expression_type(typed_expr->body);
        }
        else if constexpr(jtl::is_same<T, expr::if_>)
        {
          return expression_type(typed_expr->then);
        }
        else if constexpr(jtl::is_same<T, expr::do_>)
        {
          if(typed_expr->values.empty())
          {
            return untyped_object_ptr_type();
          }
          return expression_type(typed_expr->values.back());
        }
        else
        {
          return untyped_object_ptr_type();
        }
      },
      expr);
  }

  jtl::ptr<void> expression_scope(expression_ref const expr)
  {
    return visit_expr(
      [](auto const typed_expr) -> jtl::ptr<void> {
        using T = typename decltype(typed_expr)::value_type;

        if constexpr(jtl::is_same<T, expr::cpp_value>)
        {
          return typed_expr->scope;
        }
        else
        {
          return nullptr;
        }
      },
      expr);
  }

  /* Void is a special case which gets turned into nil, but only in some circumstances. */
  jtl::ptr<void> non_void_expression_type(expression_ref const expr)
  {
    auto const type{ expression_type(expr) };
    jank_debug_assert(type);
    if(Cpp::IsVoid(type))
    {
      return untyped_object_ptr_type();
    }
    return type;
  }

  jtl::string_result<std::vector<Cpp::TemplateArgInfo>>
  find_best_arg_types_with_conversions(std::vector<void *> const &fns,
                                       std::vector<Cpp::TemplateArgInfo> const &arg_types,
                                       bool const is_member_call)
  {
    auto const member_offset{ (is_member_call ? 1 : 0) };
    auto const arg_count{ arg_types.size() - member_offset };
    usize max_arg_count{};
    std::vector<void *> matching_fns;

    /* First rule out any fns not matching the arity we've specified. However, note that C++
     * fns can have default arguments which needn't be specified. */
    for(auto const fn : fns)
    {
      auto const num_args{ Cpp::GetFunctionNumArgs(fn) };
      if(Cpp::GetFunctionRequiredArgs(fn) <= arg_count && arg_count <= num_args)
      {
        matching_fns.emplace_back(fn);
        max_arg_count = std::max<usize>(max_arg_count, num_args);
      }
    }

    std::vector<Cpp::TemplateArgInfo> converted_args{ arg_types };

    /* If any arg can be implicitly converted to multiple functions, we have an ambiguity.
     * The user will need to specify the correct type by using a cast. */
    for(usize arg_idx{}; arg_idx < max_arg_count; ++arg_idx)
    {
      /* If our input argument here isn't an object ptr, there's no implicit conversion
       * we're going to consider. Skip to the next argument. */
      auto const arg_type{ Cpp::GetNonReferenceType(arg_types[arg_idx + member_offset].m_Type) };
      auto const is_arg_untyped_obj{ is_untyped_object(arg_type) };
      auto const is_arg_typed_obj{ is_typed_object(arg_type) };
      auto const is_arg_obj{ is_arg_untyped_obj || is_arg_typed_obj };

      jtl::option<usize> needed_conversion;
      for(usize fn_idx{}; fn_idx < matching_fns.size(); ++fn_idx)
      {
        auto const param_type{ Cpp::GetFunctionArgType(matching_fns[fn_idx], arg_idx) };
        if(!param_type)
        {
          continue;
        }
        if(is_implicitly_convertible(arg_types[arg_idx + member_offset].m_Type, param_type))
        {
          continue;
        }
        auto const is_param_obj{ is_untyped_object(param_type) || is_typed_object(param_type) };
        if(!is_arg_obj && !is_param_obj)
        {
          continue;
        }

        auto const trait_type{ is_arg_obj ? param_type : arg_type };
        if(is_trait_convertible(trait_type))
        {
          if(needed_conversion.is_some())
          {
            /* TODO: Show possible matches. */
            return err("No normal overload match was found. When considering automatic trait "
                       "conversions, this call is ambiguous.");
          }
          needed_conversion = fn_idx;
          converted_args[arg_idx + member_offset] = param_type;
        }
      }
    }

    return ok(std::move(converted_args));
  }

  jtl::string_result<jtl::ptr<void>>
  find_best_overload(std::vector<void *> const &fns,
                     std::vector<Cpp::TemplateArgInfo> &arg_types,
                     std::vector<Cpp::TCppScope_t> const &arg_scopes)
  {
    if(fns.empty())
    {
      return ok(nullptr);
    }
    jank_debug_assert(arg_types.size() == arg_scopes.size());

    auto matches{ Cpp::BestOverloadMatch(fns, arg_types, arg_scopes) };
    if(!matches.empty())
    {
      auto const match{ matches[0] };
      if(matches.size() != 1)
      {
        /* TODO: Show all matches. */
        return err("This call is ambiguous.");
      }

      auto const member{ is_non_static_member_function(match) };
      if(Cpp::IsFunctionDeleted(match))
      {
        /* TODO: Would be great to point at the C++ source for where it's deleted. */
        return err(util::format("Unable to call '{}' since it's deleted.", Cpp::GetName(match)));
      }
      if(Cpp::IsPrivateMethod(match))
      {
        return err(
          util::format("The '{}' function is private. It can only be accessed if it's public.",
                       Cpp::GetName(match)));
      }
      if(Cpp::IsProtectedMethod(match))
      {
        return err(
          util::format("The '{}' function is protected. It can only be accessed if it's public.",
                       Cpp::GetName(match)));
      }

      /* It's possible that we instantiated some unresolved templates during overload resolution.
       * To handle this, we modify the input arg types to convey their new type. Most cases, this
       * won't be different, but some design patterns require this.
       *
       * An example is `std::cout << std::endl`, since `endl` is actually a function template
       * which is parameterized on the char type and char traits. We don't have those until we
       * try to find an overload. */
      for(size_t i{ 0 }; i < arg_types.size() - member; ++i)
      {
        auto const scope{ arg_scopes[i + member] };
        if(scope)
        {
          if(Cpp::IsTemplate(scope))
          {
            auto const new_type{ Cpp::GetFunctionArgType(match, i) };
            arg_types[i + member].m_Type = new_type;
          }
        }
      }

      return match;
    }
    return ok(nullptr);
  }

  /* TODO: Cache result. */
  bool is_trait_convertible(jtl::ptr<void> const type)
  {
    static auto const convert_template{ Cpp::GetScopeFromCompleteName("jank::runtime::convert") };
    Cpp::TemplateArgInfo const arg{ Cpp::GetCanonicalType(
      Cpp::GetTypeWithoutCv(Cpp::GetNonReferenceType(type))) };
    clang::Sema::SFINAETrap const trap{ runtime::__rt_ctx->jit_prc.interpreter->getSema(), true };
    Cpp::TCppScope_t instantiation{};
    {
      auto &diag{ runtime::__rt_ctx->jit_prc.interpreter->getCompilerInstance()->getDiagnostics() };
      auto old_client{ diag.takeClient() };
      diag.setClient(new clang::IgnoringDiagConsumer{}, true);
      util::scope_exit const finally{ [&] { diag.setClient(old_client.release(), true); } };
      instantiation = Cpp::InstantiateTemplate(convert_template, &arg, 1);
    }
    return !trap.hasErrorOccurred() && Cpp::IsComplete(instantiation);
  }

  usize offset_to_typed_object_base(jtl::ptr<void> const type)
  {
    jank_debug_assert(is_typed_object(type));
    auto const can_type{ Cpp::GetCanonicalType(type) };
    auto const scope{ Cpp::GetUnderlyingScope(
      Cpp::GetNamed("value_type", Cpp::GetScopeFromType(can_type))) };
    jank_debug_assert(scope);
    auto const base{ Cpp::LookupDatamember("base", scope) };
    jank_debug_assert(base);
    auto const offset{ Cpp::GetVariableOffset(base, scope) };
    return offset;
  }

  jtl::option<Cpp::Operator> match_operator(jtl::immutable_string const &name)
  {
    static native_unordered_map<jtl::immutable_string, Cpp::Operator> const operators{
      {    "+",                Cpp::OP_Plus },
      {    "-",               Cpp::OP_Minus },
      {    "*",                Cpp::OP_Star },
      {    "/",               Cpp::OP_Slash },
      {    "%",             Cpp::OP_Percent },
      {   "++",            Cpp::OP_PlusPlus },
      {   "--",          Cpp::OP_MinusMinus },
      {   "==",          Cpp::OP_EqualEqual },
      {   "!=",        Cpp::OP_ExclaimEqual },
      {    ">",             Cpp::OP_Greater },
      {    "<",                Cpp::OP_Less },
      {   ">=",        Cpp::OP_GreaterEqual },
      {   "<=",           Cpp::OP_LessEqual },
      {  "<=>",           Cpp::OP_Spaceship },
      {    "!",             Cpp::OP_Exclaim },
      {   "&&",              Cpp::OP_AmpAmp },
      {   "||",            Cpp::OP_PipePipe },
      {    "~",               Cpp::OP_Tilde },
      {    "&",                 Cpp::OP_Amp },
      {    "|",                Cpp::OP_Pipe },
      {    "^",               Cpp::OP_Caret },
      {   "<<",            Cpp::OP_LessLess },
      {   ">>",      Cpp::OP_GreaterGreater },
      {    "=",               Cpp::OP_Equal },
      {   "+=",           Cpp::OP_PlusEqual },
      {   "-=",          Cpp::OP_MinusEqual },
      {   "*=",           Cpp::OP_StarEqual },
      {   "/=",          Cpp::OP_SlashEqual },
      {   "%=",        Cpp::OP_PercentEqual },
      {   "&=",            Cpp::OP_AmpEqual },
      {   "|=",           Cpp::OP_PipeEqual },
      {   "^=",          Cpp::OP_CaretEqual },
      {  "<<=",       Cpp::OP_LessLessEqual },
      {  ">>=", Cpp::OP_GreaterGreaterEqual },
      { "aget",           Cpp::OP_Subscript },
      /* This is not accessible through jank's syntax, but we use it internally. */
      {   "()",                Cpp::OP_Call }
    };

    auto const op{ operators.find(name) };
    if(op != operators.end())
    {
      return op->second;
    }
    return none;
  }

  jtl::option<jtl::immutable_string> operator_name(Cpp::Operator const op)
  {
    static native_unordered_map<Cpp::Operator, jtl::immutable_string> const operators{
      {                Cpp::OP_Plus,    "+" },
      {               Cpp::OP_Minus,    "-" },
      {                Cpp::OP_Star,    "*" },
      {               Cpp::OP_Slash,    "/" },
      {             Cpp::OP_Percent,    "%" },
      {            Cpp::OP_PlusPlus,   "++" },
      {          Cpp::OP_MinusMinus,   "--" },
      {          Cpp::OP_EqualEqual,   "==" },
      {        Cpp::OP_ExclaimEqual,   "!=" },
      {             Cpp::OP_Greater,    ">" },
      {                Cpp::OP_Less,    "<" },
      {        Cpp::OP_GreaterEqual,   ">=" },
      {           Cpp::OP_LessEqual,   "<=" },
      {           Cpp::OP_Spaceship,  "<=>" },
      {             Cpp::OP_Exclaim,    "!" },
      {              Cpp::OP_AmpAmp,   "&&" },
      {            Cpp::OP_PipePipe,   "||" },
      {               Cpp::OP_Tilde,    "~" },
      {                 Cpp::OP_Amp,    "&" },
      {                Cpp::OP_Pipe,    "|" },
      {               Cpp::OP_Caret,    "^" },
      {            Cpp::OP_LessLess,   "<<" },
      {      Cpp::OP_GreaterGreater,   ">>" },
      {               Cpp::OP_Equal,    "=" },
      {           Cpp::OP_PlusEqual,   "+=" },
      {          Cpp::OP_MinusEqual,   "-=" },
      {           Cpp::OP_StarEqual,   "*=" },
      {          Cpp::OP_SlashEqual,   "/=" },
      {        Cpp::OP_PercentEqual,   "%=" },
      {            Cpp::OP_AmpEqual,   "&=" },
      {           Cpp::OP_PipeEqual,   "|=" },
      {          Cpp::OP_CaretEqual,   "^=" },
      {       Cpp::OP_LessLessEqual,  "<<=" },
      { Cpp::OP_GreaterGreaterEqual,  ">>=" },
      {           Cpp::OP_Subscript, "aget" },
      {                Cpp::OP_Call,   "()" }
    };

    auto const name{ operators.find(op) };
    if(name != operators.end())
    {
      return name->second;
    }
    return none;
  }

  jtl::result<void, error_ref> ensure_convertible(expression_ref const expr)
  {
    auto const type{ expression_type(expr) };
    if(!is_any_object(type) && !is_trait_convertible(type))
    {
      return error::analyze_invalid_conversion(
        util::format("This function is returning a native object of type '{}', which is not "
                     "convertible to a jank runtime object.",
                     Cpp::GetTypeAsString(type)));
    }
    return ok();
  }

  /* By the time we get here, we know that the types are compatible in *some* fashion. This
   * is because we only apply this after using Clang to match types in the first place.
   * However, Clang doesn't tell us what needs to happen, so we then need to figure that
   * out ourselves. The benefit, though, is that we're not checking *if* two types are compatible.
   * We're checking *how* they're compatible. In other words, we're checking what we need to
   * do in order to make them compatible, with the knowledge that there's something we can do. */
  implicit_conversion_action
  determine_implicit_conversion(jtl::ptr<void> expr_type, jtl::ptr<void> const expected_type)
  {
    //util::println("determine_implicit_conversion expr type {} (canon {}), expected type {} (canon "
    //              "{}), underlying "
    //              "subclass {}",
    //              Cpp::GetTypeAsString(expr_type),
    //              Cpp::GetTypeAsString(Cpp::GetCanonicalType(expr_type)),
    //              Cpp::GetTypeAsString(expected_type),
    //              Cpp::GetTypeAsString(Cpp::GetCanonicalType(expected_type)),
    //              Cpp::IsTypeDerivedFrom(Cpp::GetUnderlyingType(expr_type),
    //                                     Cpp::GetUnderlyingType(expected_type)));

    expr_type = Cpp::GetNonReferenceType(expr_type);
    if(Cpp::GetCanonicalType(expr_type) == Cpp::GetCanonicalType(expected_type)
       || (Cpp::GetCanonicalType(expr_type)
           == Cpp::GetCanonicalType(Cpp::GetTypeWithConst(expected_type)))
       || (Cpp::GetCanonicalType(Cpp::GetTypeWithConst(expr_type))
           == Cpp::GetCanonicalType(Cpp::GetNonReferenceType(expected_type)))
       || Cpp::IsTypeDerivedFrom(Cpp::GetCanonicalType(expr_type),
                                 Cpp::GetCanonicalType(expected_type))
       || (cpp_util::is_untyped_object(expr_type) && cpp_util::is_untyped_object(expected_type)))
    {
      return implicit_conversion_action::none;
    }

    if(cpp_util::is_untyped_object(expected_type) && cpp_util::is_typed_object(expr_type))
    {
      return implicit_conversion_action::into_object;
    }

    if(cpp_util::is_typed_object(expected_type) && cpp_util::is_untyped_object(expr_type))
    {
      return implicit_conversion_action::from_object;
    }

    if(cpp_util::is_any_object(expected_type) && cpp_util::is_trait_convertible(expr_type))
    {
      return implicit_conversion_action::into_object;
    }

    if(cpp_util::is_any_object(expr_type) && cpp_util::is_trait_convertible(expected_type))
    {
      return implicit_conversion_action::from_object;
    }

    if(/* Up cast. */
       (Cpp::IsTypeDerivedFrom(Cpp::GetUnderlyingType(expr_type),
                               Cpp::GetUnderlyingType(expected_type)))
       /* Same type or adding reference. */
       || (Cpp::GetCanonicalType(expr_type)
             == Cpp::GetCanonicalType(Cpp::GetNonReferenceType(expected_type))
           && !Cpp::IsReferenceType(expr_type) && Cpp::IsReferenceType(expected_type))
       /* Matching nullptr to any pointer type. */
       || (cpp_util::is_nullptr(expr_type) && Cpp::IsPointerType(expected_type))
       /* TODO: Array size. */
       || (Cpp::IsArrayType(expr_type) && Cpp::IsArrayType(expected_type)
           && Cpp::GetArrayElementType(expr_type) == Cpp::GetArrayElementType(expected_type)))
    {
      return implicit_conversion_action::none;
    }

    if((Cpp::IsPointerType(expr_type) || Cpp::IsArrayType(expr_type))
       && (Cpp::IsPointerType(expected_type) || Cpp::IsArrayType(expected_type)))
    {
      auto const res{ determine_implicit_conversion(Cpp::GetPointeeType(expr_type),
                                                    Cpp::GetPointeeType(expected_type)) };
      switch(res)
      {
        case implicit_conversion_action::none:
          return res;
        case implicit_conversion_action::unknown:
        case implicit_conversion_action::into_object:
        case implicit_conversion_action::from_object:
        case implicit_conversion_action::cast:
          return implicit_conversion_action::unknown;
      }
    }

    if(Cpp::IsConstructible(expected_type, expr_type))
    {
      return implicit_conversion_action::cast;
    }

    return implicit_conversion_action::unknown;
  }
}
