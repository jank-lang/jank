#include <jank/runtime/sequence_range.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/util/fmt.hpp>

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
        }
      },
      data);
  }

  sequence_range::iterator::value_type sequence_range::iterator::operator*() const
  {
    return data;
  }

  sequence_range::iterator::pointer sequence_range::iterator::operator->()
  {
    return &data;
  }

  sequence_range::iterator &sequence_range::iterator::operator++()
  {
    visit_seqable(
      [&](auto const typed_data) {
        using T = typename decltype(typed_data)::value_type;

        if constexpr(behavior::sequenceable<T>)
        {
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

  sequence_range::iterator sequence_range::begin()
  {
    return { s };
  }

  sequence_range::iterator sequence_range::end()
  {
    return { obj::nil::nil_const() };
  }
}
