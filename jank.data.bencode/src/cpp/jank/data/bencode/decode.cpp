#include <stack>

/* https://wiki.theory.org/BitTorrentSpecification#Bencoding */
namespace jank::data::bencode::decode
{
  using namespace jank;
  using namespace jank::runtime;

  enum class decode_error_reason
  {
    invalid_data,
    incomplete_data
  };

  struct decode_error
  {
    native_persistent_string message;
    decode_error_reason reason{};
  };

  using decode_result = result<std::pair<object_ptr, size_t>, decode_error>;

  struct partially_decoded_list
  {
    obj::transient_vector_ptr data;
  };

  struct partially_decoded_dictionary
  {
    obj::transient_hash_map_ptr data;
    object_ptr next_key{};
  };

  using partially_decoded_collection
    = boost::variant<partially_decoded_list, partially_decoded_dictionary>;

  template <typename T>
  result<void, decode_error> append(partially_decoded_collection &coll, T const o)
  {
    if(coll.which() == 0)
    {
      auto &list(boost::get<partially_decoded_list>(coll));
      list.data = list.data->cons_in_place(o);
      return ok();
    }
    else
    {
      auto &dict(boost::get<partially_decoded_dictionary>(coll));
      if constexpr(std::same_as<T, obj::persistent_string_ptr>)
      {
        if(!dict.next_key)
        {
          dict.next_key = o;
          return ok();
        }
      }
      else
      {
        if(!dict.next_key)
        {
          return decode_error{ "non-string dict key", decode_error_reason::invalid_data };
        }
      }

      dict.data = dict.data->assoc_in_place(dict.next_key, o);
      dict.next_key = nullptr;
      return ok();
    }
  }

  result<object_ptr, decode_error> finish(partially_decoded_collection &coll)
  {
    if(coll.which() == 0)
    {
      auto &list(boost::get<partially_decoded_list>(coll));
      return ok(list.data->to_persistent());
    }
    else
    {
      auto &dict(boost::get<partially_decoded_dictionary>(coll));
      if(dict.next_key)
      {
        return decode_error{ "odd number of dict fields", decode_error_reason::invalid_data };
      }
      return ok(dict.data->to_persistent());
    }
  }

  decode_result decode_some(object_ptr const str_obj)
  {
    auto const &str(runtime::detail::to_string(str_obj));
    auto const str_len(str.size());
    size_t consumed_bytes{};

    enum class collection_type
    {
      list,
      dictionary
    };

    std::stack<partially_decoded_collection> stack;

    while(consumed_bytes < str_len)
    {
      switch(str[consumed_bytes])
      {
        case 'i':
          {
            ++consumed_bytes;
            auto const end(str.find('e', consumed_bytes));
            if(end == native_persistent_string::npos)
            {
              return decode_error{ "EOF while reading int", decode_error_reason::incomplete_data };
            }

            auto const data(str.data());
            native_integer i{};
            auto const res(std::from_chars(data + consumed_bytes, data + end, i));
            if(res.ec != std::errc{} || res.ptr != data + end)
            {
              /* TODO: Use ec value for better message. */
              return decode_error{ "unable to parse int", decode_error_reason::invalid_data };
            }
            consumed_bytes += (end - consumed_bytes) + 1;

            if(stack.empty())
            {
              return std::make_pair(make_box(i), consumed_bytes);
            }

            auto const append_res(append(stack.top(), make_box(i)));
            if(append_res.is_err())
            {
              return append_res.expect_err();
            }
          }
          break;

        case 'l':
          {
            ++consumed_bytes;
            stack.emplace(partially_decoded_list{ obj::transient_vector::empty() });
          }
          break;

        case 'd':
          {
            ++consumed_bytes;
            stack.emplace(partially_decoded_dictionary{ obj::transient_hash_map::empty() });
          }
          break;

        case 'e':
          {
            ++consumed_bytes;
            if(stack.empty())
            {
              return decode_error{ "extraneous 'e' found", decode_error_reason::invalid_data };
            }

            auto const res(finish(stack.top()));
            if(res.is_err())
            {
              return res.expect_err();
            }

            if(stack.size() == 1)
            {
              return std::make_pair(res.expect_ok(), consumed_bytes);
            }
            stack.pop();

            auto const append_res(append(stack.top(), res.expect_ok()));
            if(append_res.is_err())
            {
              return append_res.expect_err();
            }
          }
          break;

        case '0' ... '9':
          {
            auto const end(str.find(':', consumed_bytes));
            if(end == native_persistent_string::npos)
            {
              return decode_error{ "EOF while reading string",
                                   decode_error_reason::incomplete_data };
            }

            auto const data(str.data());
            native_integer size{};
            auto const res(std::from_chars(data + consumed_bytes, data + end, size));
            if(res.ec != std::errc{} || res.ptr != data + end)
            {
              /* TODO: Use ec value for better message. */
              return decode_error{ "unable to parse string size",
                                   decode_error_reason::invalid_data };
            }
            consumed_bytes += (end - consumed_bytes) + 1;

            if(size < 0 || size > str.size() - consumed_bytes)
            {
              return decode_error{ "invalid string size", decode_error_reason::invalid_data };
            }
            native_persistent_string_view s{ data + consumed_bytes, data + consumed_bytes + size };

            consumed_bytes += size;

            if(stack.empty())
            {
              return std::make_pair(make_box(s), consumed_bytes);
            }

            auto const append_res(append(stack.top(), make_box(s)));
            if(append_res.is_err())
            {
              return append_res.expect_err();
            }
          }
          break;

        default:
          return decode_error{ "unsupported character", decode_error_reason::invalid_data };
      }
    }

    if(!stack.empty())
    {
      return decode_error{ "unexpected EOF", decode_error_reason::incomplete_data };
    }

    return std::make_pair(obj::nil::nil_const(), consumed_bytes);
  }

  object_ptr decode(object_ptr const str)
  {
    auto const res(decode_some(str));
    if(res.is_err())
    {
      /* TODO: fmt details once fmt can be linked JIT */
      auto const err("bencode decode error: " + res.expect_err().message);
      throw std::runtime_error{ err.c_str() };
    }
    return res.expect_ok().first;
  }

  /* TODO: Make base class for ns which does this_object_ptr */
  struct __ns : behavior::callable
  {
    /* TODO: Remove const from all of these. */
    object_ptr call() const override
    {
      auto const ns(__rt_ctx->intern_ns("jank.data.bencode.decode"));
      ns->intern_var("decode")->bind_root(make_box<obj::native_function_wrapper>(&decode));
      return obj::nil::nil_const();
    }

    object_ptr this_object_ptr() const override
    {
      return obj::nil::nil_const();
    }
  };
}
