#include <nlohmann/json.hpp>

#include <clojure/data/json_native.hpp>

#include <jank/util/fmt.hpp>

#include <jank/runtime/behavior/map_like.hpp>
#include <jank/runtime/behavior/nameable.hpp>
#include <jank/runtime/context.hpp>
#include <jank/runtime/core/make_box.hpp>
#include <jank/runtime/core/truthy.hpp>
#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/runtime/obj/transient_hash_map.hpp>
#include <jank/runtime/obj/transient_vector.hpp>
#include <jank/runtime/sequence_range.hpp>
#include <jank/runtime/visit.hpp>

namespace clojure::data::json_native
{
  using namespace ::jank::runtime;

  static jtl::immutable_string resolve_key(object_ref const key, write_options const &opts)
  {
    if(truthy(opts.key_fn))
    {
      return try_object<obj::persistent_string>(opts.key_fn.call(key))->data;
    }

    if(key.get_type() == object_type::symbol)
    {
      return expect_object<obj::symbol>(key)->get_name();
    }

    if(key.get_type() == object_type::keyword)
    {
      return expect_object<obj::keyword>(key)->get_name();
    }

    return key.to_string();
  }

  static nlohmann::json write(object_ref const o, write_options const &opts)
  {
    return visit_object(
      [&](auto const typed_o) -> nlohmann::json {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(std::same_as<T, obj::nil>)
        {
          return {};
        }

        if constexpr(jtl::is_any_same<T,
                                      obj::boolean,
                                      obj::persistent_string,
                                      obj::integer,
                                      obj::small_integer,
                                      obj::real,
                                      obj::small_real>)
        {
          return typed_o->data;
        }

        if constexpr(std::same_as<T, obj::ratio>)
        {
          return typed_o->to_real();
        }

        if constexpr(jtl::is_any_same<T, obj::uuid, obj::big_integer, obj::big_decimal>)
        {
          return typed_o->to_string();
        }

        if constexpr(jtl::is_any_same<T, obj::inst>)
        {
          if(truthy(opts.date_formatter))
          {
            return try_object<obj::persistent_string>(opts.date_formatter.call(o))->data;
          }
          return typed_o->to_string();
        }

        if constexpr(behavior::nameable<T>)
        {
          return typed_o->get_name();
        }

        if constexpr(behavior::map_like<T>)
        {
          nlohmann::json map{};

          for(auto const &kv : typed_o->data)
          {
            auto const value(truthy(opts.value_fn) ? opts.value_fn.call(kv.first, kv.second)
                                                   : kv.second);
            if(opts.value_fn != value)
            {
              map[resolve_key(kv.first, opts)] = write(value, opts);
            }
          }

          return map;
        }

        if constexpr(behavior::seqable<T>)
        {
          auto array(nlohmann::json::array());

          for(auto const e : make_sequence_range(typed_o))
          {
            array.push_back(write(e, opts));
          }

          return array;
        }

        throw std::runtime_error{ jank::util::format("JSON write error (unsupported type: {})",
                                                     object_type_str(typed_o.get_type())) };
      },
      o);
  }

  jtl::immutable_string write_str(object_ref const x, write_options const &opts)
  {
    if(opts.indent)
    {
      return write(x, opts).dump(2);
    }
    return write(x, opts).dump();
  }

  static object_ref read(nlohmann::json const &json, read_options const &opts)
  {
    switch(json.type())
    {
      case nlohmann::json::value_t::null:
        return jank_nil;
      case nlohmann::json::value_t::string:
        return make_box<obj::persistent_string>(json.get<std::string>());
      case nlohmann::json::value_t::number_unsigned:
      case nlohmann::json::value_t::number_integer:
        return make_box(json.get<jtl::i64>());
      case nlohmann::json::value_t::number_float:
        {
          if(opts.bigdec)
          {
            return make_box<obj::big_decimal>(json.get<jtl::f64>());
          }

          return make_box(json.get<jtl::f64>());
        }
      case nlohmann::json::value_t::boolean:
        return make_box(json.get<bool>());
      case nlohmann::json::value_t::array:
        {
          obj::transient_vector v{};

          for(auto const &element : json)
          {
            v.conj_in_place(read(element, opts));
          }

          return v.to_persistent();
        }
      case nlohmann::json::value_t::object:
        {
          obj::transient_hash_map m{};

          for(auto const &[k, v] : json.items())
          {
            auto const key_initial(make_box<obj::persistent_string>(k));
            auto const value_initial(read(v, opts));

            auto const key(truthy(opts.key_fn) ? opts.key_fn.call(key_initial) : key_initial);
            auto const value(truthy(opts.value_fn) ? opts.value_fn.call(key, value_initial)
                                                   : value_initial);
            if(opts.value_fn != value)
            {
              m.assoc_in_place(key, value);
            }
          }

          return m.to_persistent();
        }
      case nlohmann::json::value_t::binary:
        throw std::runtime_error{ "JSON read error (unsupported type: binary)" };
      case nlohmann::json::value_t::discarded:
        return opts.eof_error ? throw std::runtime_error{ "JSON read error (failed to parse)" }
                              : opts.eof_value;
    }
  }

  object_ref read_str(jtl::immutable_string const &string, read_options const &opts)
  {
    auto const json(nlohmann::json::parse(string));
    return read(json, opts);
  }
}
