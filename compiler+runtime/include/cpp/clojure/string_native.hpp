#pragma once

#include <jtl/immutable_string.hpp>

#include <jank/runtime/object.hpp>

namespace jank::runtime::obj
{
  using persistent_vector_ref = oref<struct persistent_vector>;
  using re_pattern_ref = oref<struct re_pattern>;
}

namespace clojure::string_native
{
  using namespace jank;
  using namespace jank::runtime;

  jtl::immutable_string reverse(jtl::immutable_string const &s);
  jtl::immutable_string lower_case(jtl::immutable_string const &s);
  bool starts_with(jtl::immutable_string const &s, jtl::immutable_string const &substr);
  bool ends_with(jtl::immutable_string const &s, jtl::immutable_string const &substr);
  bool includes(jtl::immutable_string const &s, jtl::immutable_string const &substr);
  jtl::immutable_string upper_case(jtl::immutable_string const &s);
  object_ref
  replace_first(object_ref const s, object_ref const match, object_ref const replacement);

  i64 index_of(jtl::immutable_string const &s,
               jtl::immutable_string const &value,
               i64 const from_index);
  i64 last_index_of(jtl::immutable_string const &s,
                    jtl::immutable_string const &value,
                    i64 const from_index);

  jtl::immutable_string triml(jtl::immutable_string const &s);
  jtl::immutable_string trimr(jtl::immutable_string const &s);
  jtl::immutable_string trim(jtl::immutable_string const &s);
  jtl::immutable_string trim_newline(jtl::immutable_string const &s);

  obj::persistent_vector_ref split(jtl::immutable_string const &s, obj::re_pattern_ref const re);
  obj::persistent_vector_ref
  split(jtl::immutable_string const &s, obj::re_pattern_ref const re, i64 const limit);
}
