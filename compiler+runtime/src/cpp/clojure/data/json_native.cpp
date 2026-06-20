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

  static nlohmann::json write(object_ref const o)
  {
    return visit_object(
      [](auto const typed_o) -> nlohmann::json {
        using T = typename decltype(typed_o)::value_type;

        if constexpr(std::same_as<T, obj::nil>)
        {
          return {};
        }

        if constexpr(std::same_as<T, obj::boolean> || std::same_as<T, obj::persistent_string>
                     || std::same_as<T, obj::integer> || std::same_as<T, obj::small_integer>
                     || std::same_as<T, obj::real> || std::same_as<T, obj::small_real>)
        {
          return typed_o->data;
        }

        if constexpr(std::same_as<T, obj::ratio>)
        {
          return typed_o->to_real();
        }

        if constexpr(jtl::is_any_same<T, obj::uuid, obj::inst, obj::big_integer, obj::big_decimal>)
        {
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
            map[write(kv.first)] = write(kv.second);
          }

          return map;
        }

        if constexpr(behavior::seqable<T>)
        {
          auto array(nlohmann::json::array());

          for(auto const e : make_sequence_range(typed_o))
          {
            array.push_back(write(e));
          }

          return array;
        }

        throw std::runtime_error{ jank::util::format("JSON write error (unsupported type: {})",
                                                     object_type_str(typed_o.get_type())) };
      },
      o);
  }

  jtl::immutable_string write_str(object_ref const x)
  {
    return write(x).dump();
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
        return make_box(json.get<jtl::f64>());
      case nlohmann::json::value_t::boolean:
        return json.get<bool>() ? jank_true : jank_false;
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
