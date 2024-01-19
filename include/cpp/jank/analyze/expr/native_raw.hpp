#pragma once

#include <boost/variant.hpp>

#include <jank/runtime/obj/persistent_string.hpp>
#include <jank/analyze/local_frame.hpp>
#include <jank/analyze/expression_base.hpp>
#include <jank/detail/to_runtime_data.hpp>
#include <jank/runtime/seq.hpp>

namespace jank::analyze::expr
{
  /* native/raw expressions start as a string of C++ code which can contain
   * interpolated jank code, but that string is split up into its various pieces
   * for easier codegen. */
  template <typename E>
  struct native_raw : expression_base
  {
    using chunk_t = boost::variant<native_persistent_string, native_box<E>>;

    native_vector<chunk_t> chunks;

    runtime::object_ptr to_runtime_data() const
    {
      runtime::object_ptr chunk_maps(make_box<runtime::obj::vector>());
      for(auto const &e : chunks)
      {
        chunk_maps = runtime::conj(chunk_maps,
                                   boost::apply_visitor(
                                     [](auto const &d) -> runtime::object_ptr {
                                       using T = std::decay_t<decltype(d)>;

                                       if constexpr(std::same_as<T, native_persistent_string>)
                                       {
                                         return make_box(d);
                                       }
                                       else
                                       {
                                         return d->to_runtime_data();
                                       }
                                     },
                                     e));
      }

      return runtime::merge(
        static_cast<expression_base const *>(this)->to_runtime_data(),
        runtime::obj::persistent_array_map::create_unique(make_box("__type"),
                                                          make_box("expr::native_raw"),
                                                          make_box("chunks"),
                                                          chunk_maps));
    }
  };
}
