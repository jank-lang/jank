#include <jank/runtime/obj/reader_conditional.hpp>
#include <jank/runtime/core/seq_ext.hpp>

namespace jank::runtime::obj
{
  reader_conditional::reader_conditional()
    : object{ obj_type, obj_behaviors }
  {
  }

  reader_conditional::reader_conditional(value_type &&d)
    : object{ obj_type, obj_behaviors }
    , data{ std::move(d) }
  {
  }

  reader_conditional::reader_conditional(value_type const &d)
    : object{ obj_type, obj_behaviors }
    , data{ d }
  {
  }

  bool reader_conditional::equal(object const &o) const
  {
    return runtime::equal(o, data.begin(), data.end());
  }

  void reader_conditional::to_string(jtl::string_builder &buff) const
  {
    runtime::to_string(data.begin(), data.end(), "#?(", ')', buff);
  }

  jtl::immutable_string reader_conditional::to_string() const
  {
    jtl::string_builder buff;
    runtime::to_string(data.begin(), data.end(), "#?(", ')', buff);
    return buff.release();
  }

  jtl::immutable_string reader_conditional::to_code_string() const
  {
    jtl::string_builder buff;
    runtime::to_code_string(data.begin(), data.end(), "#?(", ')', buff);
    return buff.release();
  }

  /* TODO: Cache this. */
  uhash reader_conditional::to_hash() const
  {
    return hash::ordered(data.begin(), data.end());
  }
}
