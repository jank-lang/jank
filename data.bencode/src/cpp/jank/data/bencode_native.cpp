namespace jank::data::bencode_native
{
  using namespace jank;
  using namespace jank::runtime;

  enum class parse_error
  {
    invalid_data,
    incomplete_data
  };

  using parse_result = result<std::pair<object_ptr, size_t>, parse_error>;

  parse_result parse_some(object_ptr const str)
  {
    return std::make_pair(obj::nil::nil_const(), 0);
  }

  object_ptr parse(object_ptr const str)
  {
    auto const res(parse_some(str));
    if(res.is_err())
    {
      /* TODO: fmt details once fmt can be linked JIT */
      throw std::runtime_error{ "bencode parse error" };
    }
    return res.expect_ok().first;
  }

  /* TODO: Make base class for ns which does this_object_ptr */
  struct __ns : behavior::callable
  {
    /* TODO: Remove const from all of these. */
    object_ptr call() const override
    {
      auto const ns(__rt_ctx->intern_ns("jank.data.bencode-native"));
      ns->intern_var("parse")->bind_root(make_box<obj::native_function_wrapper>(&parse));
      return obj::nil::nil_const();
    }

    object_ptr this_object_ptr() const override
    {
      return obj::nil::nil_const();
    }
  };
}
