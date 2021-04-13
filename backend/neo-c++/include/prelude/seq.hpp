#pragma once

#include <prelude/object.hpp>

namespace jank
{
  /* TODO: Laziness. */
  inline object mapv(object const &f, object const &seq)
  {
    return seq.visit
    (
      [&](auto &&data) -> object
      {
        using T = std::decay_t<decltype(data)>;
        /* TODO: Generic seq handling. */
        auto constexpr is_vector(std::is_same_v<T, detail::vector>);
        auto constexpr is_set(std::is_same_v<T, detail::set>);
        auto constexpr is_map(std::is_same_v<T, detail::map>);

        if constexpr(is_vector || is_set || is_map)
        {
          detail::vector ret;
          ret.reserve(data.size());

          if constexpr(is_vector || is_set)
          {
            for(auto const &e : data)
            { ret.push_back(detail::invoke(&f, e)); }
          }
          else if constexpr(is_map)
          {
            for(auto const &p : data)
            { ret.push_back(detail::invoke(&f, object{ detail::vector{ p.first, p.second } })); }
          }

          return object{ ret };
        }
        else
        {
          /* TODO: Throw an error. */
          std::cout << "not a seq" << std::endl;
          return JANK_NIL;
        }
      }
    );
  }

  inline object reduce(object const &f, object const &initial, object const &seq)
  {
    return seq.visit
    (
      [&](auto &&data) -> object
      {
        using T = std::decay_t<decltype(data)>;
        /* TODO: Generic seq handling. */
        auto constexpr is_vector(std::is_same_v<T, detail::vector>);
        auto constexpr is_set(std::is_same_v<T, detail::set>);

        if constexpr(is_vector || is_set)
        {
          object acc{ initial };

          for(auto const &e : data)
          { acc = detail::invoke(&f, acc, e); }

          return acc;
        }
        else
        {
          /* TODO: Throw an error. */
          std::cout << "not a seq" << std::endl;
          return JANK_NIL;
        }
      }
    );
  }

  /* TODO: Laziness */
  inline object partition(object const &n, object const &seq)
  {
    if(n.get_kind() != object::kind::integer)
    {
      /* TODO: throw error */
      std::cout << "partition size must be an integer" << std::endl;
      return JANK_NIL;
    }
    auto const partition_size(*n.get<detail::integer>());

    return seq.visit
    (
      [&](auto &&data) -> object
      {
        using T = std::decay_t<decltype(data)>;
        /* TODO: Generic seq handling. */
        auto constexpr is_vector(std::is_same_v<T, detail::vector>);
        /* TODO: set + map support */

        if constexpr(is_vector)
        {
          detail::vector ret;
          auto const partitions(data.size() / partition_size);
          ret.reserve(partitions);

          for(size_t i{}; i < partitions; ++i)
          {
            detail::vector partition;
            ret.reserve(partition_size);

            for(size_t k{ i * partition_size }; k < (i * partition_size) + partition_size; ++k)
            { partition.push_back(data[k]); }

            ret.emplace_back(object{ std::move(partition) });
          }

          return object{ ret };
        }
        else
        {
          /* TODO: Throw an error. */
          std::cout << "not a seq" << std::endl;
          return JANK_NIL;
        }
      }
    );
  }

  /* TODO: Laziness */
  inline object range(object const &start, object const &end)
  {
    if(start.get_kind() != object::kind::integer
       || end.get_kind() != object::kind::integer)
    {
      /* TODO: throw error */
      std::cout << "range start/end must be an integer" << std::endl;
      return JANK_NIL;
    }
    else if(end < start)
    {
      /* TODO: throw error */
      std::cout << "range start must be < end" << std::endl;
      return JANK_NIL;
    }

    auto const start_int(*start.get<detail::integer>());
    auto const end_int(*end.get<detail::integer>());

    detail::vector ret;
    ret.reserve(end_int - start_int);
    for(auto i(start_int); i < end_int; ++i)
    { ret.push_back(i); }
    return ret;
  }

  inline object reverse(object const &seq)
  {
    return seq.visit
    (
      [&](auto &&data) -> object
      {
        using T = std::decay_t<decltype(data)>;
        /* TODO: Generic seq handling. */
        auto constexpr is_vector(std::is_same_v<T, detail::vector>);

        if constexpr(is_vector)
        {
          T ret;
          ret.reserve(data.size());

          for(auto it(data.rbegin()); it != data.rend(); ++it)
          { ret.push_back(*it); }

          return ret;
        }
        else
        {
          /* TODO: Throw an error. */
          std::cout << "not a reversible seq" << std::endl;
          return JANK_NIL;
        }
      }
    );
  }

  inline object get(object const &o, object const &key)
  {
    switch(o.get_kind())
    {
      case object::kind::vector:
      case object::kind::map:
        return o.visit
        (
          [&](auto &&data) -> object
          {
            using T = std::decay_t<decltype(data)>;
            /* TODO: Generic associative handling. */
            auto constexpr is_vector(std::is_same_v<T, detail::vector>);
            auto constexpr is_map(std::is_same_v<T, detail::map>);

            if constexpr(is_vector)
            {
              if(key.get_kind() != object::kind::integer)
              { return JANK_NIL; }

              auto const i(*key.get<detail::integer>());
              if(i < 0 || i >= data.size())
              { return JANK_NIL; }

              return data[i];
            }
            else if constexpr(is_map)
            {
              auto const it(data.find(key));
              if(it == data.end())
              { return JANK_NIL; }
              return it->second;
            }
            else
            {
              /* TODO: Throw an error. */
              std::cout << "not associative" << std::endl;
              return JANK_NIL;
            }
          }
        );
        break;
      default:
      {
        /* TODO: throw error */
        std::cout << "can only call get on associative types" << std::endl;
        return JANK_NIL;
      }
    }
  }

  inline object assoc(object const &o, object const &key, object const &val)
  {
    return o.visit
    (
      [&](auto &&data) -> object
      {
        using T = std::decay_t<decltype(data)>;
        /* TODO: Generic seq handling. */
        auto constexpr is_vector(std::is_same_v<T, detail::vector>);
        auto constexpr is_map(std::is_same_v<T, detail::map>);

        if constexpr(is_vector)
        {
          if(key.get_kind() != object::kind::integer)
          {
            /* TODO: throw error */
            std::cout << "vector assoc key must be an integer: " << key << std::endl;
            return JANK_NIL;
          }

          auto const i(*key.get<detail::integer>());
          if(i < 0 || i >= data.size())
          {
            /* TODO: Throw error */
            std::cout << "vector assoc key out of bounds: " << key << std::endl;
            return JANK_NIL;
          }

          T ret{ data };
          ret[i] = val;
          return object{ ret };
        }
        else if constexpr(is_map)
        {
          T ret{ data };
          ret[key] = val;
          return object{ ret };
        }
        else
        {
          /* TODO: Throw an error. */
          std::cout << "not a seq" << std::endl;
          return JANK_NIL;
        }
      }
    );
  }
}
