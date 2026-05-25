#include <jank/runtime/core/call.hpp>
#include <jank/runtime/behavior/seqable.hpp>
#include <jank/runtime/visit.hpp>
#include <jank/runtime/core.hpp>
#include <jank/runtime/sequence_range.hpp>
#include <jank/util/make_array.hpp>
#include <jank/util/fmt.hpp>

namespace jank::runtime
{
  using namespace behavior;

  object_ref apply_to(object_ref const source, object_ref const args)
  {
    return visit_seqable(
      [=](auto const typed_args) -> object_ref {
        auto const s(typed_args->fresh_seq());
        auto const length(sequence_length(s, max_params + 1));
        auto const r{ make_sequence_range(s) };
        auto it{ r.begin() };

        /* XXX: It's UB to have multiple *(++it) in one expression, since the order in
         * which they will run is not defined. This requires us to pull out all of the
         * args into locals. */
        switch(length)
        {
          case 0:
            return source.call();
          case 1:
            {
              auto const a1{ *it };
              return source.call(a1);
            }
          case 2:
            {
              auto const a1{ *it };
              auto const a2{ *(++it) };
              return source.call(a1, a2);
            }
          case 3:
            {
              auto const a1{ *it };
              auto const a2{ *(++it) };
              auto const a3{ *(++it) };
              return source.call(a1, a2, a3);
            }
          case 4:
            {
              auto const a1{ *it };
              auto const a2{ *(++it) };
              auto const a3{ *(++it) };
              auto const a4{ *(++it) };
              return source.call(a1, a2, a3, a4);
            }
          case 5:
            {
              auto const a1{ *it };
              auto const a2{ *(++it) };
              auto const a3{ *(++it) };
              auto const a4{ *(++it) };
              auto const a5{ *(++it) };
              return source.call(a1, a2, a3, a4, a5);
            }
          case 6:
            {
              auto const a1{ *it };
              auto const a2{ *(++it) };
              auto const a3{ *(++it) };
              auto const a4{ *(++it) };
              auto const a5{ *(++it) };
              auto const a6{ *(++it) };
              return source.call(a1, a2, a3, a4, a5, a6);
            }
          case 7:
            {
              auto const a1{ *it };
              auto const a2{ *(++it) };
              auto const a3{ *(++it) };
              auto const a4{ *(++it) };
              auto const a5{ *(++it) };
              auto const a6{ *(++it) };
              auto const a7{ *(++it) };
              return source.call(a1, a2, a3, a4, a5, a6, a7);
            }
          case 8:
            {
              auto const a1{ *it };
              auto const a2{ *(++it) };
              auto const a3{ *(++it) };
              auto const a4{ *(++it) };
              auto const a5{ *(++it) };
              auto const a6{ *(++it) };
              auto const a7{ *(++it) };
              auto const a8{ *(++it) };
              return source.call(a1, a2, a3, a4, a5, a6, a7, a8);
            }
          case 9:
            {
              auto const a1{ *it };
              auto const a2{ *(++it) };
              auto const a3{ *(++it) };
              auto const a4{ *(++it) };
              auto const a5{ *(++it) };
              auto const a6{ *(++it) };
              auto const a7{ *(++it) };
              auto const a8{ *(++it) };
              auto const a9{ *(++it) };
              return source.call(a1, a2, a3, a4, a5, a6, a7, a8, a9);
            }
          case 10:
            {
              auto const a1{ *it };
              auto const a2{ *(++it) };
              auto const a3{ *(++it) };
              auto const a4{ *(++it) };
              auto const a5{ *(++it) };
              auto const a6{ *(++it) };
              auto const a7{ *(++it) };
              auto const a8{ *(++it) };
              auto const a9{ *(++it) };
              auto const a10{ *(++it) };
              return source.call(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
            }
          default:
            {
              auto const a1{ *it };
              auto const a2{ *(++it) };
              auto const a3{ *(++it) };
              auto const a4{ *(++it) };
              auto const a5{ *(++it) };
              auto const a6{ *(++it) };
              auto const a7{ *(++it) };
              auto const a8{ *(++it) };
              auto const a9{ *(++it) };
              auto const a10{ *(++it) };
              return source.call(a1,
                                 a2,
                                 a3,
                                 a4,
                                 a5,
                                 a6,
                                 a7,
                                 a8,
                                 a9,
                                 a10,
                                 obj::persistent_list::create((++it).data));
            }
        }
      },
      args);
  }
}
