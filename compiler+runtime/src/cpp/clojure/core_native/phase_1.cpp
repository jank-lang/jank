#include <functional>

#include <jank/runtime/convert.hpp>

namespace clojure::core_native::phase_1
{
  using namespace jank;
  using namespace jank::runtime;

  object_ptr subvec(object_ptr const o, object_ptr const start, object_ptr const end)
  {
    return runtime::subvec(o, runtime::to_int(start), runtime::to_int(end));
  }

  object_ptr not_(object_ptr const o)
  {
    if(runtime::is_nil(o))
    {
      return obj::boolean::true_const();
    }
    return make_box(runtime::is_false(o));
  }

  object_ptr to_unqualified_symbol(object_ptr const o)
  {
    return runtime::visit_object(
      [&](auto const typed_o) -> object_ptr {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(std::same_as<T, obj::symbol>)
        {
          return typed_o;
        }
        else if constexpr(std::same_as<T, obj::persistent_string>)
        {
          return make_box<obj::symbol>(typed_o->data);
        }
        else if constexpr(std::same_as<T, var>)
        {
          return typed_o->name;
        }
        else if constexpr(std::same_as<T, obj::keyword>)
        {
          return make_box<obj::symbol>(typed_o->sym);
        }
        else
        {
          throw std::runtime_error{ fmt::format("can't convert {} to a symbol",
                                                typed_o->to_code_string()) };
        }
      },
      o);
  }

  object_ptr to_qualified_symbol(object_ptr const ns, object_ptr const name)
  {
    return make_box<obj::symbol>(ns, name);
  }

  object_ptr lazy_seq(object_ptr const o)
  {
    return make_box<obj::lazy_sequence>(o);
  }

  object_ptr is_var(object_ptr const o)
  {
    return make_box(o->type == object_type::var);
  }
}

jank_object_ptr jank_load_clojure_core_native_phase_1()
{
  using namespace jank;
  using namespace jank::runtime;
  using namespace clojure::core_native;

  auto const ns(__rt_ctx->intern_ns("clojure.core-native.phase-1"));

  auto const intern_fn([=](native_persistent_string const &name, auto const fn) {
    ns->intern_var(name)->bind_root(
      make_box<obj::native_function_wrapper>(convert_function(fn))
        ->with_meta(obj::persistent_hash_map::create_unique(
          std::make_pair(__rt_ctx->intern_keyword("clojure.core-native", "name").expect_ok(),
                         make_box(name)))));
  });
  auto const intern_fn_obj([=](native_persistent_string const &name, object_ptr const fn) {
    ns->intern_var(name)->bind_root(
      with_meta(fn,
                obj::persistent_hash_map::create_unique(std::make_pair(
                  __rt_ctx->intern_keyword("clojure.core-native", "name").expect_ok(),
                  make_box(name)))));
  });

  intern_fn("type", &type);
  intern_fn("nil?", &is_nil);
  intern_fn("identical?", &is_identical);
  intern_fn("empty?", &is_empty);
  intern_fn("empty", &empty);
  intern_fn("count", static_cast<size_t (*)(object_ptr)>(&sequence_length));
  intern_fn("boolean", static_cast<native_bool (*)(object_ptr)>(&truthy));
  intern_fn("integer", static_cast<native_integer (*)(object_ptr)>(&to_int));
  intern_fn("real", static_cast<native_real (*)(object_ptr)>(&to_real));
  intern_fn("seq", static_cast<object_ptr (*)(object_ptr)>(&seq));
  intern_fn("fresh-seq", static_cast<object_ptr (*)(object_ptr)>(&fresh_seq));
  intern_fn("first", static_cast<object_ptr (*)(object_ptr)>(&first));
  intern_fn("second", static_cast<object_ptr (*)(object_ptr)>(&second));
  intern_fn("next", static_cast<object_ptr (*)(object_ptr)>(&next));
  intern_fn("next-in-place", static_cast<object_ptr (*)(object_ptr)>(&next_in_place));
  intern_fn("rest", static_cast<object_ptr (*)(object_ptr)>(&rest));
  intern_fn("cons", &cons);
  intern_fn("coll?", &is_collection);
  intern_fn("seq?", &is_seq);
  intern_fn("list?", &is_list);
  intern_fn("vector?", &is_vector);
  intern_fn("vec", &vec);
  intern_fn("subvec", &phase_1::subvec);
  intern_fn("conj", &conj);
  intern_fn("map?", &is_map);
  intern_fn("assoc", &assoc);
  intern_fn("pr-str", static_cast<native_persistent_string (*)(object const *)>(&to_code_string));
  intern_fn("string?", &is_string);
  intern_fn("to-string", static_cast<native_persistent_string (*)(object const *)>(&to_string));
  intern_fn("str", static_cast<native_persistent_string (*)(object_ptr, object_ptr)>(&str));
  intern_fn("symbol?", &is_symbol);
  intern_fn("true?", &is_true);
  intern_fn("false?", &is_false);
  intern_fn("not", &phase_1::not_);
  intern_fn("some?", &is_some);
  intern_fn("meta", &meta);
  intern_fn("with-meta", &with_meta);
  intern_fn("reset-meta!", &reset_meta);
  intern_fn("macroexpand-1", &macroexpand1);
  intern_fn("macroexpand", &macroexpand);
  intern_fn("->unqualified-symbol", &phase_1::to_unqualified_symbol);
  intern_fn("->qualified-symbol", &phase_1::to_qualified_symbol);
  intern_fn("apply*", &apply_to);
  intern_fn("transientable?", &is_transientable);
  intern_fn("transient", &transient);
  intern_fn("persistent!", &persistent);
  intern_fn("conj-in-place!", &conj_in_place);
  intern_fn("assoc-in-place!", &assoc_in_place);
  intern_fn("dissoc-in-place!", &dissoc_in_place);
  intern_fn("pop-in-place!", &pop_in_place);
  intern_fn("disj-in-place!", &disj_in_place);
  intern_fn("apply-to", &apply_to);
  intern_fn("deref", &deref);
  intern_fn("reduced", &reduced);
  intern_fn("reduced?", &is_reduced);
  intern_fn("reduce", &reduce);
  intern_fn("peek", &peek);
  intern_fn("pop", &pop);
  intern_fn("atom", &atom);
  intern_fn("compare-and-set!", &compare_and_set);
  intern_fn("reset!", &reset);
  intern_fn("reset-vals!", &reset_vals);
  intern_fn("volatile!", &volatile_);
  intern_fn("volatile?", &is_volatile);
  intern_fn("vreset!", &vreset);
  intern_fn("vswap!", &vswap);
  intern_fn("+", static_cast<object_ptr (*)(object_ptr, object_ptr)>(&add));
  intern_fn("-", static_cast<object_ptr (*)(object_ptr, object_ptr)>(&sub));
  intern_fn("/", static_cast<object_ptr (*)(object_ptr, object_ptr)>(&div));
  intern_fn("*", static_cast<object_ptr (*)(object_ptr, object_ptr)>(&mul));
  intern_fn("bit-not", &bit_not);
  intern_fn("bit-and", &bit_and);
  intern_fn("bit-or", &bit_or);
  intern_fn("bit-xor", &bit_xor);
  intern_fn("bit-and-not", &bit_and_not);
  intern_fn("bit-clear", &bit_clear);
  intern_fn("bit-set", &bit_set);
  intern_fn("bit-flip", &bit_flip);
  intern_fn("bit-test", &bit_test);
  intern_fn("bit-shift-left", &bit_shift_left);
  intern_fn("bit-shift-right", &bit_shift_right);
  intern_fn("unsigned-bit-shift-right", &bit_unsigned_shift_right);
  intern_fn("<", static_cast<native_bool (*)(object_ptr, object_ptr)>(&lt));
  intern_fn("<=", static_cast<native_bool (*)(object_ptr, object_ptr)>(&lte));
  intern_fn("min", static_cast<object_ptr (*)(object_ptr, object_ptr)>(&min));
  intern_fn("max", static_cast<object_ptr (*)(object_ptr, object_ptr)>(&max));
  intern_fn("inc", static_cast<object_ptr (*)(object_ptr)>(&inc));
  intern_fn("dec", static_cast<object_ptr (*)(object_ptr)>(&dec));
  intern_fn("pos?", &is_pos);
  intern_fn("neg?", &is_neg);
  intern_fn("zero?", &is_zero);
  intern_fn("rem", static_cast<object_ptr (*)(object_ptr, object_ptr)>(&rem));
  intern_fn("integer?", &is_integer);
  intern_fn("real?", &is_real);
  intern_fn("boolean?", &is_boolean);
  intern_fn("number?", &is_number);
  intern_fn("even?", &is_even);
  intern_fn("odd?", &is_odd);
  intern_fn("rand", &runtime::rand);
  intern_fn("sequential?", &is_sequential);
  intern_fn("first-index-of", &first_index_of);
  intern_fn("last-index-of", &last_index_of);
  intern_fn("lazy-seq*", &phase_1::lazy_seq);
  intern_fn("chunk-buffer", &chunk_buffer);
  intern_fn("chunk-append", &chunk_append);
  intern_fn("chunk", &chunk);
  intern_fn("chunk-first", &chunk_first);
  intern_fn("chunk-next", &chunk_next);
  intern_fn("chunk-rest", &chunk_rest);
  intern_fn("chunk-cons", &chunk_cons);
  intern_fn("chunk-seq?", &is_chunked_seq);
  intern_fn("dissoc", &dissoc);
  intern_fn("contains?", &contains);
  intern_fn("find", &find);
  intern_fn("disj", &disj);
  intern_fn("hash", &to_hash);
  intern_fn("set?", &is_set);
  intern_fn("named?", &is_named);
  intern_fn("name", &name);
  intern_fn("namespace", &namespace_);
  intern_fn("var?", &phase_1::is_var);
  intern_fn("push-thread-bindings", &push_thread_bindings);
  intern_fn("pop-thread-bindings", &pop_thread_bindings);
  intern_fn("get-thread-bindings", &get_thread_bindings);
  intern_fn("keyword?", &is_keyword);
  intern_fn("simple-keyword?", &is_simple_keyword);
  intern_fn("qualified-keyword?", &is_qualified_keyword);
  intern_fn("keyword", &keyword);
  intern_fn("simple-symbol?", &is_simple_symbol);
  intern_fn("qualified-symbol?", &is_qualified_symbol);
  intern_fn("iterate", &iterate);

  {
    auto const fn(
      make_box<obj::jit_function>(behavior::callable::build_arity_flags(0, true, false)));
    fn->arity_1 = [](object * const seq) -> object * { return list(seq); };
    intern_fn_obj("list", fn);
  }

  {
    auto const fn(
      make_box<obj::jit_function>(behavior::callable::build_arity_flags(2, true, true)));
    fn->arity_1 = [](object *) -> object * { return obj::boolean::true_const(); };
    fn->arity_2
      = [](object * const l, object * const r) -> object * { return make_box(equal(l, r)); };
    fn->arity_3 = [](object * const l, object * const r, object * const rest) -> object * {
      if(!equal(l, r))
      {
        return obj::boolean::false_const();
      }

      for(auto it(fresh_seq(rest)); it != nullptr; it = next_in_place(it))
      {
        if(!equal(l, first(it)))
        {
          return obj::boolean::false_const();
        }
      }

      return obj::boolean::true_const();
    };
    intern_fn_obj("=", fn);
  }

  {
    auto const fn(
      make_box<obj::jit_function>(behavior::callable::build_arity_flags(2, true, true)));
    fn->arity_1 = [](object *) -> object * { return obj::boolean::true_const(); };
    fn->arity_2
      = [](object * const l, object * const r) -> object * { return make_box(is_equiv(l, r)); };
    fn->arity_3 = [](object * const l, object * const r, object * const rest) -> object * {
      if(!is_equiv(l, r))
      {
        return obj::boolean::false_const();
      }

      for(auto it(fresh_seq(rest)); it != nullptr; it = next_in_place(it))
      {
        if(!is_equiv(l, first(it)))
        {
          return obj::boolean::false_const();
        }
      }

      return obj::boolean::true_const();
    };
    intern_fn_obj("==", fn);
  }

  {
    auto const fn(
      make_box<obj::jit_function>(behavior::callable::build_arity_flags(0, true, false)));
    fn->arity_1 = [](object * const seq) -> object * { return println(seq); };
    intern_fn_obj("println", fn);
  }

  {
    auto const fn(
      make_box<obj::jit_function>(behavior::callable::build_arity_flags(0, true, false)));
    fn->arity_1 = [](object * const seq) -> object * { return print(seq); };
    intern_fn_obj("print", fn);
  }

  {
    auto const fn(
      make_box<obj::jit_function>(behavior::callable::build_arity_flags(0, true, false)));
    fn->arity_1 = [](object * const seq) -> object * { return prn(seq); };
    intern_fn_obj("prn", fn);
  }

  {
    auto const fn(
      make_box<obj::jit_function>(behavior::callable::build_arity_flags(0, true, false)));
    fn->arity_1 = [](object * const seq) -> object * { return pr(seq); };
    intern_fn_obj("pr", fn);
  }

  {
    auto const fn(
      make_box<obj::jit_function>(behavior::callable::build_arity_flags(0, false, false)));
    fn->arity_0 = []() -> object * { return gensym(make_box("G__")); };
    fn->arity_1 = [](object * const prefix) -> object * { return gensym(prefix); };
    intern_fn_obj("gensym", fn);
  }

  {
    auto const fn(
      make_box<obj::jit_function>(behavior::callable::build_arity_flags(4, true, true)));
    fn->arity_2 = [](object * const atom, object * const fn) -> object * {
      return try_object<obj::atom>(atom)->swap(fn);
    };
    fn->arity_3 = [](object * const atom, object * const fn, object * const a1) -> object * {
      return try_object<obj::atom>(atom)->swap(fn, a1);
    };
    fn->arity_4 =
      [](object * const atom, object * const fn, object * const a1, object * const a2) -> object * {
      return try_object<obj::atom>(atom)->swap(fn, a1, a2);
    };
    fn->arity_5 = [](object * const atom,
                     object * const fn,
                     object * const a1,
                     object * const a2,
                     object * const rest) -> object * {
      return try_object<obj::atom>(atom)->swap(fn, a1, a2, rest);
    };
    intern_fn_obj("swap!", fn);
  }

  {
    auto const fn(
      make_box<obj::jit_function>(behavior::callable::build_arity_flags(4, true, true)));
    fn->arity_2 = [](object * const atom, object * const fn) -> object * {
      return try_object<obj::atom>(atom)->swap_vals(fn);
    };
    fn->arity_3 = [](object * const atom, object * const fn, object * const a1) -> object * {
      return try_object<obj::atom>(atom)->swap_vals(fn, a1);
    };
    fn->arity_4 =
      [](object * const atom, object * const fn, object * const a1, object * const a2) -> object * {
      return try_object<obj::atom>(atom)->swap_vals(fn, a1, a2);
    };
    fn->arity_5 = [](object * const atom,
                     object * const fn,
                     object * const a1,
                     object * const a2,
                     object * const rest) -> object * {
      return try_object<obj::atom>(atom)->swap_vals(fn, a1, a2, rest);
    };
    intern_fn_obj("swap-vals!", fn);
  }

  {
    auto const fn(
      make_box<obj::jit_function>(behavior::callable::build_arity_flags(0, false, false)));
    fn->arity_2 = [](object * const s, object * const start) -> object * { return subs(s, start); };
    fn->arity_3 = [](object * const s, object * const start, object * const end) -> object * {
      return subs(s, start, end);
    };
    intern_fn_obj("subs", fn);
  }

  {
    auto const fn(
      make_box<obj::jit_function>(behavior::callable::build_arity_flags(0, true, true)));
    fn->arity_0 = []() -> object * { return obj::persistent_hash_map::empty(); };
    fn->arity_1 = [](object * const kvs) -> object * {
      return obj::persistent_hash_map::create_from_seq(kvs);
    };
    intern_fn_obj("hash-map", fn);
  }

  {
    auto const fn(
      make_box<obj::jit_function>(behavior::callable::build_arity_flags(0, true, true)));
    fn->arity_0 = []() -> object * { return obj::persistent_sorted_map::empty(); };
    fn->arity_1 = [](object * const kvs) -> object * {
      return obj::persistent_sorted_map::create_from_seq(kvs);
    };
    intern_fn_obj("sorted-map", fn);
  }

  {
    auto const fn(
      make_box<obj::jit_function>(behavior::callable::build_arity_flags(0, true, true)));
    fn->arity_0 = []() -> object * { return obj::persistent_hash_set::empty(); };
    fn->arity_1 = [](object * const kvs) -> object * {
      return obj::persistent_hash_set::create_from_seq(kvs);
    };
    intern_fn_obj("hash-set", fn);
  }

  {
    auto const fn(
      make_box<obj::jit_function>(behavior::callable::build_arity_flags(0, true, true)));
    fn->arity_0 = []() -> object * { return obj::persistent_sorted_set::empty(); };
    fn->arity_1 = [](object * const kvs) -> object * {
      return obj::persistent_sorted_set::create_from_seq(kvs);
    };
    intern_fn_obj("sorted-set", fn);
  }

  {
    auto const fn(
      make_box<obj::jit_function>(behavior::callable::build_arity_flags(3, false, false)));
    fn->arity_2 = [](object * const o, object * const k) -> object * { return get(o, k); };
    fn->arity_3 = [](object * const o, object * const k, object * const fallback) -> object * {
      return get(o, k, fallback);
    };
    intern_fn_obj("get", fn);
  }

  {
    auto const fn(
      make_box<obj::jit_function>(behavior::callable::build_arity_flags(3, false, false)));
    fn->arity_2 = [](object * const o, object * const k) -> object * { return get_in(o, k); };
    fn->arity_3 = [](object * const o, object * const k, object * const fallback) -> object * {
      return get_in(o, k, fallback);
    };
    intern_fn_obj("get-in", fn);
  }

  {
    auto const fn(
      make_box<obj::jit_function>(behavior::callable::build_arity_flags(3, false, false)));
    fn->arity_0 = []() -> object * {
      return iterate(__rt_ctx->intern_var("clojure.core", "inc").expect_ok()->deref(), make_box(0));
    };
    fn->arity_1 = [](object * const end) -> object * { return obj::range::create(end); };
    fn->arity_2 = [](object * const start, object * const end) -> object * {
      return obj::range::create(start, end);
    };
    fn->arity_3 = [](object * const start, object * const end, object * const step) -> object * {
      return obj::range::create(start, end, step);
    };
    intern_fn_obj("range", fn);
  }

  {
    auto const fn(
      make_box<obj::jit_function>(behavior::callable::build_arity_flags(3, false, false)));
    fn->arity_0 = []() -> object * {
      return iterate(__rt_ctx->intern_var("clojure.core", "inc").expect_ok()->deref(), make_box(0));
    };
    fn->arity_2
      = [](object * const coll, object * const index) -> object * { return nth(coll, index); };
    fn->arity_3
      = [](object * const coll, object * const index, object * const fallback) -> object * {
      return nth(coll, index, fallback);
    };
    intern_fn_obj("nth", fn);
  }

  return erase(obj::nil::nil_const());
}
