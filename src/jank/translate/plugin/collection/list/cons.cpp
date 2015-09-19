#include <algorithm>

#include <jank/interpret/cell/stream.hpp>
#include <jank/translate/plugin/collection/list/cons.hpp>
#include <jank/translate/plugin/detail/make_function.hpp>
#include <jank/translate/environment/builtin/type/primitive.hpp>
#include <jank/translate/environment/builtin/type/list.hpp>
#include <jank/translate/environment/builtin/value/primitive.hpp>
#include <jank/interpret/environment/resolve_value.hpp>
#include <jank/interpret/expect/type.hpp>

namespace jank
{
  namespace translate
  {
    namespace plugin
    {
      namespace collection
      {
        namespace list
        {
          static void make_cons
          (
            std::shared_ptr<environment::scope> const &scope,
            cell::detail::type_reference<cell::cell> const &elem_type,
            cell::detail::type_reference<cell::cell> const &col_type
          )
          {
            (void)scope;
            (void)elem_type;
            (void)col_type;
            //plugin::detail::make_function
            //(
            //  scope, "cons",
            //  [](auto const &scope, auto const &args)
            //  {
            //    auto elem
            //    (
            //      interpret::expect::type<interpret::cell::type::integer>
            //      (interpret::environment::resolve_value(scope, args[0].cell)).data
            //    );
            //    auto coll
            //    (
            //      interpret::expect::type<interpret::cell::type::list>
            //      (interpret::environment::resolve_value(scope, args[1].cell)).data
            //    );
            //    coll.push_front(interpret::cell::integer{ elem });
            //    std::list<parse::cell::integer> ret_coll;
            //    std::transform
            //    (
            //      coll.begin(), coll.end(),
            //      std::back_inserter(ret_coll),
            //      [](auto const &int_cell)
            //      {
            //        auto const i
            //        (
            //          interpret::expect::type<interpret::cell::type::integer>
            //          (int_cell).data
            //        );
            //        return parse::cell::integer{ i };
            //      }
            //    );
            //    return cell::cell{ cell::literal_value{ ret_coll } };
            //  },
            //  col_type,
            //  elem_type, col_type
            //);

            //plugin::detail::make_function
            //(
            //  scope, "first",
            //  [](auto const &scope, auto const &args)
            //  {
            //    auto coll
            //    (
            //      interpret::expect::type<interpret::cell::type::list>
            //      (interpret::environment::resolve_value(scope, args[0].cell)).data
            //    );
            //    auto const i
            //    (
            //      interpret::expect::type<interpret::cell::type::integer>
            //      (coll.front()).data
            //    );
            //    return cell::cell{ cell::literal_value{ parse::cell::integer{ i } } };
            //  },
            //  elem_type,
            //  col_type
            //);

            //plugin::detail::make_function
            //(
            //  scope, "rest",
            //  [](auto const &scope, auto const &args)
            //  {
            //    auto coll
            //    (
            //      interpret::expect::type<interpret::cell::type::list>
            //      (interpret::environment::resolve_value(scope, args[0].cell)).data
            //    );
            //    coll.pop_front();
            //    std::list<parse::cell::integer> ret_coll;
            //    std::transform
            //    (
            //      coll.begin(), coll.end(),
            //      std::back_inserter(ret_coll),
            //      [](auto const &int_cell)
            //      {
            //        auto const i
            //        (
            //          interpret::expect::type<interpret::cell::type::integer>
            //          (int_cell).data
            //        );
            //        return parse::cell::integer{ i };
            //      }
            //    );
            //    return cell::cell{ cell::literal_value{ ret_coll } };
            //  },
            //  col_type,
            //  col_type
            //);

            //plugin::detail::make_function
            //(
            //  scope, "count",
            //  [](auto const &scope, auto const &args)
            //  {
            //    auto coll
            //    (
            //      interpret::expect::type<interpret::cell::type::list>
            //      (interpret::environment::resolve_value(scope, args[0].cell)).data
            //    );
            //    return cell::cell
            //    {
            //      cell::literal_value
            //      {
            //        parse::cell::integer
            //        { static_cast<parse::cell::integer::type>(coll.size()) }
            //      }
            //    };
            //  },
            //  elem_type,
            //  col_type
            //);

            ///* TODO: Remove */
            //plugin::detail::make_function
            //(
            //  scope, "cons",
            //  [](auto const &, auto const &)
            //  {
            //    std::list<parse::cell::integer> ret_coll;
            //    return cell::cell{ cell::literal_value{ ret_coll } };
            //  },
            //  col_type
            //);
          }

          void cons(std::shared_ptr<environment::scope> const &scope)
          {
            make_cons
            (
              scope,
              environment::builtin::type::integer(*scope),
              environment::builtin::type::list
              (
                *scope,
                environment::builtin::type::integer(*scope)
              )
            );
          }
        }
      }
    }
  }
}
