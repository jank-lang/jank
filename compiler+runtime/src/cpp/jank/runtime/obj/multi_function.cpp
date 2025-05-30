#include <jank/runtime/obj/multi_function.hpp>
#include <jank/runtime/obj/persistent_hash_set.hpp>
#include <jank/runtime/obj/persistent_vector_sequence.hpp>
#include <jank/runtime/obj/persistent_hash_map.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/rtti.hpp>
#include <jank/runtime/core.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime::obj
{
  multi_function::multi_function(object_ref const name,
                                 object_ref const dispatch,
                                 object_ref const default_,
                                 object_ref const hierarchy)
    : dispatch{ dispatch }
    , default_dispatch_value{ default_ }
    , hierarchy{ hierarchy }
    , method_table{ persistent_hash_map::empty() }
    , method_cache{ persistent_hash_map::empty() }
    , prefer_table{ persistent_hash_map::empty() }
    , name{ try_object<symbol>(name) }
  {
  }

  bool multi_function::equal(object const &rhs) const
  {
    return &base == &rhs;
  }

  jtl::immutable_string multi_function::to_string()
  {
    util::string_builder buff;
    to_string(buff);
    return buff.release();
  }

  void multi_function::to_string(util::string_builder &buff)
  {
    util::format_to(buff, "{} ({}@{})", name->to_string(), object_type_str(base.type), &base);
  }

  jtl::immutable_string multi_function::to_code_string()
  {
    return to_string();
  }

  uhash multi_function::to_hash() const
  {
    return static_cast<uhash>(reinterpret_cast<uintptr_t>(this));
  }

  object_ref multi_function::call()
  {
    return dynamic_call(get_fn(dynamic_call(dispatch)));
  }

  object_ref multi_function::call(object_ref const a1)
  {
    return dynamic_call(get_fn(dynamic_call(dispatch, a1)), a1);
  }

  object_ref multi_function::call(object_ref const a1, object_ref const a2)
  {
    return dynamic_call(get_fn(dynamic_call(dispatch, a1, a2)), a1, a2);
  }

  object_ref multi_function::call(object_ref const a1, object_ref const a2, object_ref const a3)
  {
    return dynamic_call(get_fn(dynamic_call(dispatch, a1, a2, a3)), a1, a2, a3);
  }

  object_ref multi_function::call(object_ref const a1,
                                  object_ref const a2,
                                  object_ref const a3,
                                  object_ref const a4)
  {
    return dynamic_call(get_fn(dynamic_call(dispatch, a1, a2, a3, a4)), a1, a2, a3, a4);
  }

  object_ref multi_function::call(object_ref const a1,
                                  object_ref const a2,
                                  object_ref const a3,
                                  object_ref const a4,
                                  object_ref const a5)
  {
    return dynamic_call(get_fn(dynamic_call(dispatch, a1, a2, a3, a4, a5)), a1, a2, a3, a4, a5);
  }

  object_ref multi_function::call(object_ref const a1,
                                  object_ref const a2,
                                  object_ref const a3,
                                  object_ref const a4,
                                  object_ref const a5,
                                  object_ref const a6)
  {
    return dynamic_call(get_fn(dynamic_call(dispatch, a1, a2, a3, a4, a5, a6)),
                        a1,
                        a2,
                        a3,
                        a4,
                        a5,
                        a6);
  }

  object_ref multi_function::call(object_ref const a1,
                                  object_ref const a2,
                                  object_ref const a3,
                                  object_ref const a4,
                                  object_ref const a5,
                                  object_ref const a6,
                                  object_ref const a7)
  {
    return dynamic_call(get_fn(dynamic_call(dispatch, a1, a2, a3, a4, a5, a6, a7)),
                        a1,
                        a2,
                        a3,
                        a4,
                        a5,
                        a6,
                        a7);
  }

  object_ref multi_function::call(object_ref const a1,
                                  object_ref const a2,
                                  object_ref const a3,
                                  object_ref const a4,
                                  object_ref const a5,
                                  object_ref const a6,
                                  object_ref const a7,
                                  object_ref const a8)
  {
    return dynamic_call(get_fn(dynamic_call(dispatch, a1, a2, a3, a4, a5, a6, a7, a8)),
                        a1,
                        a2,
                        a3,
                        a4,
                        a5,
                        a6,
                        a7,
                        a8);
  }

  object_ref multi_function::call(object_ref const a1,
                                  object_ref const a2,
                                  object_ref const a3,
                                  object_ref const a4,
                                  object_ref const a5,
                                  object_ref const a6,
                                  object_ref const a7,
                                  object_ref const a8,
                                  object_ref const a9)
  {
    return dynamic_call(get_fn(dynamic_call(dispatch, a1, a2, a3, a4, a5, a6, a7, a8, a9)),
                        a1,
                        a2,
                        a3,
                        a4,
                        a5,
                        a6,
                        a7,
                        a8,
                        a9);
  }

  object_ref multi_function::call(object_ref const a1,
                                  object_ref const a2,
                                  object_ref const a3,
                                  object_ref const a4,
                                  object_ref const a5,
                                  object_ref const a6,
                                  object_ref const a7,
                                  object_ref const a8,
                                  object_ref const a9,
                                  object_ref const a10)
  {
    return dynamic_call(get_fn(dynamic_call(dispatch, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)),
                        a1,
                        a2,
                        a3,
                        a4,
                        a5,
                        a6,
                        a7,
                        a8,
                        a9,
                        a10);
  }

  object_ref multi_function::this_object_ref()
  {
    return &this->base;
  }

  multi_function_ref multi_function::reset()
  {
    std::lock_guard<std::recursive_mutex> const locked{ data_lock };
    cached_hierarchy = jank_nil;
    method_table = prefer_table = method_cache = persistent_hash_map::empty();
    return this;
  }

  persistent_hash_map_ref multi_function::reset_cache()
  {
    std::lock_guard<std::recursive_mutex> const locked{ data_lock };
    cached_hierarchy = hierarchy;
    method_cache = method_table;
    return method_cache;
  }

  multi_function_ref
  multi_function::add_method(object_ref const dispatch_val, object_ref const method)
  {
    std::lock_guard<std::recursive_mutex> const locked{ data_lock };

    method_table = method_table->assoc(dispatch_val, method);
    reset_cache();
    return this;
  }

  multi_function_ref multi_function::remove_method(object_ref const dispatch_val)
  {
    std::lock_guard<std::recursive_mutex> const locked{ data_lock };
    method_table = method_table->dissoc(dispatch_val);
    reset_cache();
    return this;
  }

  multi_function_ref multi_function::prefer_method(object_ref const x, object_ref const y)
  {
    std::lock_guard<std::recursive_mutex> const locked{ data_lock };

    if(is_preferred(deref(hierarchy), y, x))
    {
      throw std::runtime_error{ util::format(
        "Preference conflict in multimethod '{}': {} is already preferred to {}",
        runtime::to_string(name),
        runtime::to_string(y),
        runtime::to_string(x)) };
    }

    prefer_table = prefer_table->assoc(
      x,
      runtime::conj(runtime::get(prefer_table, x, persistent_hash_set::empty()), y));
    reset_cache();
    return this;
  }

  bool multi_function::is_preferred(object_ref const hierarchy,
                                    object_ref const x,
                                    object_ref const y) const
  {
    auto const x_prefs(prefer_table->get(x));
    if(x_prefs != jank_nil && expect_object<persistent_hash_set>(x_prefs)->contains(y))
    {
      return true;
    }

    static object_ref const parents{
      __rt_ctx->intern_var("clojure.core", "parents").expect_ok()->deref()
    };

    for(auto it(fresh_seq(dynamic_call(parents, hierarchy, y))); it != jank_nil;
        it = next_in_place(it))
    {
      if(is_preferred(hierarchy, x, first(it)))
      {
        return true;
      }
    }

    for(auto it(fresh_seq(dynamic_call(parents, hierarchy, x))); it != jank_nil;
        it = next_in_place(it))
    {
      if(is_preferred(hierarchy, first(it), y))
      {
        return true;
      }
    }

    return false;
  }

  bool multi_function::is_a(object_ref const hierarchy, object_ref const x, object_ref const y)
  {
    static object_ref const isa{
      __rt_ctx->intern_var("clojure.core", "isa?").expect_ok()->deref()
    };
    return truthy(dynamic_call(isa, deref(hierarchy), x, y));
  }

  bool multi_function::is_dominant(object_ref const hierarchy,
                                   object_ref const x,
                                   object_ref const y) const
  {
    return is_preferred(hierarchy, x, y) || is_a(hierarchy, x, y);
  }

  object_ref multi_function::get_fn(object_ref const dispatch_val)
  {
    auto const target(get_method(dispatch_val));
    if(target == jank_nil)
    {
      throw std::runtime_error{ util::format("No method in multimethod '{}' for dispatch value: {}",
                                             runtime::to_string(name),
                                             runtime::to_string(dispatch_val)) };
    }
    return target;
  }

  object_ref multi_function::get_method(object_ref const dispatch_val)
  {
    if(cached_hierarchy != deref(hierarchy))
    {
      reset_cache();
    }

    auto const target(method_cache->get(dispatch_val));
    if(target != jank_nil)
    {
      return target;
    }

    return find_and_cache_best_method(dispatch_val);
  }

  object_ref multi_function::find_and_cache_best_method(object_ref const dispatch_val)
  {
    /* TODO: Clojure uses a RW lock here for better parallelism. */
    std::lock_guard<std::recursive_mutex> const locked{ data_lock };
    object_ref best_value{ jank_nil };
    persistent_vector_sequence_ref best_entry{};

    for(auto it(method_table->fresh_seq()); it.is_some(); it = it->next_in_place())
    {
      auto const entry(it->first());
      auto const entry_key(entry->seq()->first());

      if(is_a(cached_hierarchy, dispatch_val, entry_key))
      {
        if(best_entry.is_nil() || is_dominant(cached_hierarchy, entry_key, best_entry->first()))
        {
          best_entry = entry->seq();
        }

        if(!is_dominant(cached_hierarchy, best_entry->first(), entry_key))
        {
          throw std::runtime_error{ util::format(
            "Multiple methods in multimethod '{}' match dispatch value: {} -> {} and {}, and "
            "neither is preferred",
            runtime::to_string(name),
            runtime::to_string(dispatch_val),
            runtime::to_string(entry_key),
            runtime::to_string(best_entry->first())) };
        }
      }
    }

    if(best_entry.is_some())
    {
      best_value = second(best_entry);
    }
    else
    {
      best_value = method_table->get(default_dispatch_value);
      if(best_value == jank_nil)
      {
        return best_value;
      }
    }

    method_cache = method_cache->assoc(dispatch_val, best_value);

    return best_value;
  }
}
