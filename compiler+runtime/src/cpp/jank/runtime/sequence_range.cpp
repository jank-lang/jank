#include <jank/runtime/sequence_range.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core/seq.hpp>
#include <jank/util/fmt/print.hpp>

namespace jank::runtime
{
  sequence_range::iterator::iterator(object_ref const data)
    : data{ data }
  {
    visit_seqable(
      [&](auto const typed_data) {
        using T = typename decltype(typed_data)::value_type;

        if constexpr(requires(T t) { t.fresh_seq(); })
        {
          this->data = typed_data->fresh_seq().erase();
          in_place = true;
        }
        else
        {
          this->data = typed_data->seq().erase();
        }
      },
      data);
  }

  sequence_range::iterator::value_type sequence_range::iterator::operator*() const
  {
    return runtime::first(data);
  }

  sequence_range::iterator::pointer sequence_range::iterator::operator->() const
  {
    return runtime::first(data);
  }

  sequence_range::iterator &sequence_range::iterator::operator++()
  {
    visit_seqable(
      [&](auto const typed_data) {
        using T = typename decltype(typed_data)::value_type;

        if constexpr(behavior::sequenceable<T>)
        {
          /* If we didn't grab a fresh seq to start with, it doesn't matter if the current
           * object is updatable in place. We don't own it. */
          if(in_place)
          {
            data = typed_data->next();
            return;
          }

          if constexpr(requires(T t) { t.next_in_place(); })
          {
            data = typed_data->next_in_place();
          }
          else
          {
            data = typed_data->next();
          }
        }
        else
        {
          throw std::runtime_error{ jank::util::format("invalid sequence: {}",
                                                       typed_data->to_string()) };
        }
      },
      data);

    return *this;
  }

  bool sequence_range::iterator::operator!=(sequence_range::iterator const &rhs) const
  {
    return data != rhs.data;
  }

  bool sequence_range::iterator::operator==(sequence_range::iterator const &rhs) const
  {
    return data == rhs.data;
  }

  sequence_range::iterator sequence_range::begin() const
  {
    return { s };
  }

  sequence_range::iterator sequence_range::end() const
  {
    return { obj::nil::nil_const() };
  }

  sequence_range sequence_range::skip(usize const n) const
  {
    sequence_range r{ s };
    auto it{ r.begin() };
    for(usize i{}; i < n; ++i)
    {
      ++it;
    }

    return { it.data };
  }
}
